#include "timefield-render-module.h"

bool TimefieldRenderModule::StartUp()
{
	_HoldRenderLayer.create(3840, 2160);
	_NoteRenderLayer.create(3840, 2160);

	_SegmentedRenderTexture.create(3840, 2160);


	_OnScreenNotes.reserve(1000);

	return true;
}

bool TimefieldRenderModule::Tick(const float& InDeltaTime) 
{
	_Skin.UpdateTimefieldMetrics(_TimefieldMetrics);

	return true;
}

bool TimefieldRenderModule::RenderBack(sf::RenderTarget* const InOutRenderTarget) 
{
	_Skin.RenderTimeFieldBackground(InOutRenderTarget);

	return true;
}

bool TimefieldRenderModule::RenderFront(sf::RenderTarget* const InOutRenderTarget) 
{
	return false;
}

void TimefieldRenderModule::RenderTimefieldGraph(sf::RenderTarget* const InOutRenderTarget, TimefieldRenderGraph& InOutTimefieldRenderGraph, const Time InTime, const float InZoomLevel, const bool InRegisterToOnscreenNotes) 
{
	if(InRegisterToOnscreenNotes)
		_OnScreenNotes.clear();

	_HoldRenderLayer.clear({ 0,0,0,0 });
	_NoteRenderLayer.clear({ 0,0,0,0 });

	InOutTimefieldRenderGraph.Render([this, &InTime, &InZoomLevel, &InRegisterToOnscreenNotes](const NoteRenderCommand& InNoteRenderCommand)
	{
		const Note& note = InNoteRenderCommand.RenderNote;
		const Column column = InNoteRenderCommand.NoteColumn;

		const int y = GetScreenPointFromTime(note.TimePoint, InTime, InZoomLevel) + _TimefieldMetrics.NoteScreenPivot;

		//hold pass
		switch (note.Type)
		{
		case Note::EType::HoldBegin:
		case Note::EType::HoldIntermediate:
		case Note::EType::HoldEnd:
			{
				int endY = GetScreenPointFromTime(note.TimePointEnd, InTime, InZoomLevel) - _TimefieldMetrics.ColumnSize / 2;
				int height = GetScreenPointFromTime(note.TimePointBegin, InTime, InZoomLevel) - endY;

				_Skin.RenderHoldBody(column, endY + _TimefieldMetrics.NoteScreenPivot, height, &_HoldRenderLayer, InNoteRenderCommand.Alpha);
			}
		}

		//common pass
		switch (note.Type)
		{
		case Note::EType::Common:
		case Note::EType::HoldBegin:
			_Skin.RenderNote(column, y, &_NoteRenderLayer, note.BeatSnap, InNoteRenderCommand.Alpha);
			break;

		case Note::EType::HoldEnd:
			_Skin.RenderHoldCap(column, y, &_HoldRenderLayer, InNoteRenderCommand.Alpha);
			break;
		}

		if(InRegisterToOnscreenNotes)
			_OnScreenNotes.push_back({&note, column, y});
	});

	_HoldRenderLayer.display();
	_NoteRenderLayer.display();

	_HoldRenderLayerSprite.setTexture(_HoldRenderLayer.getTexture());
	_NoteRenderLayerSprite.setTexture(_NoteRenderLayer.getTexture());

	_HoldRenderLayerSprite.setPosition(0, 0);
	_NoteRenderLayerSprite.setPosition(0, 0);

	InOutRenderTarget->draw(_HoldRenderLayerSprite);
	InOutRenderTarget->draw(_NoteRenderLayerSprite);

	InOutTimefieldRenderGraph.Render([this, &InOutRenderTarget, &InTime, &InZoomLevel](const TimefieldRenderCommand& InRenderCommand)
	{
		InRenderCommand.RenderWork(InOutRenderTarget, _TimefieldMetrics, _TimefieldMetrics.FirstColumnPosition + InRenderCommand.ColumnPoint * _TimefieldMetrics.ColumnSize, GetScreenPointFromTime(InRenderCommand.TimePoint, InTime, InZoomLevel));
	});
}

sf::RenderTexture* const TimefieldRenderModule::GetRenderedTimefieldGraphSegment(TimefieldRenderGraph& InOutTimefieldRenderGraph, const Time InTime, const float InZoomLevel)
{
	_SegmentedRenderTexture.clear({0, 0, 0, 0});
	_ResultingSegmentedRenderTexture.clear({0, 0, 0, 0});

	RenderTimefieldGraph(&_SegmentedRenderTexture, InOutTimefieldRenderGraph, InTime, InZoomLevel, false);

	sf::IntRect segment;

	segment.width = _ResultingSegmentedRenderTexture.getSize().x;
	segment.height = _ResultingSegmentedRenderTexture.getSize().y;

	segment.top = 0;
	segment.left = _TimefieldMetrics.FirstColumnPosition;

	_SegmentedSprite.setTexture(_SegmentedRenderTexture.getTexture());
	_SegmentedSprite.setTextureRect(segment);
	_SegmentedSprite.setPosition(0, 0);

	_ResultingSegmentedRenderTexture.draw(_SegmentedSprite);

	return &_ResultingSegmentedRenderTexture;
}

void TimefieldRenderModule::RenderBeatLine(sf::RenderTarget* const InOutRenderTarget, const Time InBeatTimePoint, const int InBeatSnap, const Time InTime, const float InZoomLevel) 
{
	sf::RectangleShape line(sf::Vector2f(_TimefieldMetrics.FieldWidth, 1));
	line.setPosition(_TimefieldMetrics.LeftSidePosition, GetScreenPointFromTime(InBeatTimePoint, InTime, InZoomLevel));
	
	line.setFillColor(_Skin.SnapColorTable[InBeatSnap]);

	InOutRenderTarget->draw(line);
}

void TimefieldRenderModule::RenderReceptors(sf::RenderTarget* const InOutRenderTarget, const int InBeatSnap) 
{
	_Skin.RenderReceptors(InOutRenderTarget, InBeatSnap);
}


int TimefieldRenderModule::GetScreenPointFromTime(const Time InTimePoint, const Time InTime, const float InZoomLevel)
{
	int timePoint = InTimePoint - InTime;

	timePoint = float(timePoint) * InZoomLevel + 0.5f;
	timePoint += _TimefieldMetrics.HitLine;

	timePoint = _WindowMetrics.Height - timePoint;

	return timePoint;
}

Time TimefieldRenderModule::GetTimeFromScreenPoint(const int InScreenPointY, const Time InTime, const float InZoomLevel, const bool InNoteTimePivot)
{
	int screenPoint = _WindowMetrics.Height - InScreenPointY + ((int)InNoteTimePivot * _TimefieldMetrics.NoteScreenPivot);

	screenPoint -= _TimefieldMetrics.HitLine;
	screenPoint = float(screenPoint) / InZoomLevel - 0.5f;

	const int timePoint = screenPoint + InTime;

	return timePoint;
}

Time TimefieldRenderModule::GetWindowTimePointBegin(const Time InTime, const float InZoomLevel)
{
	return GetTimeFromScreenPoint(_WindowMetrics.Height, InTime, InZoomLevel);
}

Time TimefieldRenderModule::GetWindowTimePointEnd(const Time InTime, const float InZoomLevel)
{
	return GetTimeFromScreenPoint(0, InTime, InZoomLevel);
}

Column TimefieldRenderModule::GetColumnFromScreenPoint(const int InScreenPointX) 
{
	int inputX = InScreenPointX - _TimefieldMetrics.ColumnSize;
	
	const int leftBorder = _WindowMetrics.MiddlePoint - _TimefieldMetrics.NoteFieldWidthHalf;
	const int rightBorder = _WindowMetrics.MiddlePoint + _TimefieldMetrics.NoteFieldWidthHalf;

	const int noteWidth = _TimefieldMetrics.ColumnSize;

	if (inputX < leftBorder)
		inputX = leftBorder;

	if (inputX > rightBorder)
		inputX = rightBorder;

	for (Column column = 0; column < _TimefieldMetrics.KeyAmount; column++)
	{
		if (inputX >= leftBorder + noteWidth * column && inputX < leftBorder + noteWidth + noteWidth * column)
			return column;
	}

	return  _TimefieldMetrics.KeyAmount - 1;
}

void TimefieldRenderModule::GetOverlappedOnScreenNotes(const Column InColumn, const int InScreenPointY, std::vector<const Note*>& OutNoteCollection) 
{
	for(auto& onScreenNote : _OnScreenNotes)
	{
		if(InColumn != onScreenNote.Column)
			continue;

		if(!(InScreenPointY >= onScreenNote.OnScreenY - _TimefieldMetrics.ColumnSize && InScreenPointY <= onScreenNote.OnScreenY))
			continue;

		OutNoteCollection.push_back(onScreenNote.Note);
	}
}

Skin& TimefieldRenderModule::GetSkin() 
{
	return _Skin;
}

const TimefieldMetrics& TimefieldRenderModule::GetTimefieldMetrics() 
{
	return _TimefieldMetrics;
}

void TimefieldRenderModule::SetKeyAmount(const int InKeyAmount) 
{
	_KeyAmount = InKeyAmount;

	_TimefieldMetrics.KeyAmount = _KeyAmount;
	_TimefieldMetrics.NoteScreenPivot = _TimefieldMetrics.NoteScreenPivotsLookup[_KeyAmount];

	_TimefieldMetrics.FieldWidth = _TimefieldMetrics.ColumnSize * _TimefieldMetrics.KeyAmount + _TimefieldMetrics.SideSpace * 2;
	_TimefieldMetrics.FieldWidthHalf = _TimefieldMetrics.FieldWidth / 2;
	
	_TimefieldMetrics.NoteFieldWidth = _TimefieldMetrics.ColumnSize * _TimefieldMetrics.KeyAmount;
	_TimefieldMetrics.NoteFieldWidthHalf = _TimefieldMetrics.FieldWidth / 2;

	_ResultingSegmentedRenderTexture.create(_TimefieldMetrics.NoteFieldWidth, 2160);

	_Skin.LoadResources(InKeyAmount);
}

void TimefieldRenderModule::UpdateMetrics(const WindowMetrics& InWindowMetrics) 
{
	_WindowMetrics = InWindowMetrics;

	_TimefieldMetrics.LeftSidePosition = _WindowMetrics.MiddlePoint - _TimefieldMetrics.FieldWidthHalf;
	_TimefieldMetrics.LeftSidePositionFromCenter = -_TimefieldMetrics.FieldWidthHalf;

	_TimefieldMetrics.FirstColumnPosition = _TimefieldMetrics.LeftSidePosition + _TimefieldMetrics.SideSpace;

	_TimefieldMetrics.HitLinePosition = _WindowMetrics.Height - _TimefieldMetrics.HitLine;
}