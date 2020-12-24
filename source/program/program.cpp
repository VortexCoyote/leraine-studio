#include "program.h"

#include "imgui.h"
#include "../utilities/imgui/addons/imgui_user.h"

//module includes
#include "../modules/imgui-module.h"
#include "../modules/chart-parser-module.h"
#include "../modules/dialog-module.h"
#include "../modules/timefield-render-module.h"
#include "../modules/audio-module.h"
#include "../modules/input-module.h"
#include "../modules/beat-module.h"
#include "../modules/edit-module.h"
#include "../modules/background-module.h"
#include "../modules/minimap-module.h"

void Program::RegisterModules()
{
	ModuleManager::Register<BackgroundModule>();
	ModuleManager::Register<ImGuiModule>();
	ModuleManager::Register<DialogModule>();
	ModuleManager::Register<InputModule>();
	ModuleManager::Register<MiniMapModule>();
	ModuleManager::Register<ChartParserModule>();
	ModuleManager::Register<AudioModule>();
	ModuleManager::Register<BeatModule>();
	ModuleManager::Register<EditModule>();
	ModuleManager::Register<TimefieldRenderModule>();
}

void Program::InnerStartUp()
{
	MOD(ImGuiModule).Init(_RenderWindow);
}

void Program::InnerTick()
{
	MenuBarRoutines();

	if (!_SelectedChart || MOD(DialogModule).IsDialogOpen())
		return;

	UpdateCursorData();
	MOD(EditModule).SetCursorData(_Cursor);
	
	InputActions();

	const Time windowTimeBegin = MOD(TimefieldRenderModule).GetWindowTimePointBegin(MOD(AudioModule).GetTimeMilliSeconds(), _ZoomLevel);
	const Time windowTimeEnd   = MOD(TimefieldRenderModule).GetWindowTimePointEnd(MOD(AudioModule).GetTimeMilliSeconds(), _ZoomLevel);

	if(MOD(MiniMapModule).IsHoveringTimeline(32, _WindowMetrics.MenuBarHeight, _WindowMetrics.Height - _WindowMetrics.MenuBarHeight, 16, MOD(AudioModule).GetTimeMilliSeconds(), windowTimeBegin, windowTimeEnd, _Cursor))
	{
		if (MOD(InputModule).WasMouseButtonPressed(sf::Mouse::Left))		
			MOD(AudioModule).SetTimeMilliSeconds((MOD(MiniMapModule).GetHoveredTime()));	
	}

	MOD(TimefieldRenderModule).UpdateMetrics(_WindowMetrics);
	MOD(BeatModule).GenerateTimeRangeBeatLines(windowTimeBegin, windowTimeEnd, _SelectedChart, _CurrentSnap);

	_SelectedChart->IterateNotesInTimeRange(windowTimeBegin - TIMESLICE_LENGTH, windowTimeEnd, [this](Note& InNote, const Column InColumn)
	{
		_ChartRenderGraph.SubmitNoteRenderCommand(InNote, InColumn);
	});

	MOD(EditModule).SubmitToRenderGraph(_PreviewRenderGraph);
}

void Program::InnerRender(sf::RenderTarget* const InOutRenderTarget)
{
	if (!_SelectedChart)
		return;

	MOD(TimefieldRenderModule).RenderTimefieldBackground(InOutRenderTarget);

	MOD(MiniMapModule).Render(InOutRenderTarget);

	MOD(BeatModule).IterateThroughBeatlines([this, &InOutRenderTarget](const BeatLine& InBeatLine)
	{
		MOD(TimefieldRenderModule).RenderBeatLine(InOutRenderTarget, InBeatLine.TimePoint, InBeatLine.BeatSnap, MOD(AudioModule).GetTimeMilliSeconds(), _ZoomLevel);
	});

	MOD(TimefieldRenderModule).RenderTimefieldGraph(InOutRenderTarget ,_ChartRenderGraph, MOD(AudioModule).GetTimeMilliSeconds(), _ZoomLevel);
	MOD(TimefieldRenderModule).RenderTimefieldGraph(InOutRenderTarget, _PreviewRenderGraph, MOD(AudioModule).GetTimeMilliSeconds(), _ZoomLevel, false);

	MOD(TimefieldRenderModule).RenderTimefieldForeground(InOutRenderTarget);
}

void Program::InnerShutDown()
{
	
}

//************************************************************************************************************************************************************************************

void Program::MenuBarRoutines()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open", "CTRL+O"))
			{
				MOD(DialogModule).OpenFolderDialog([this](const std::string& InPath)
				{
					_SelectedChartSet = MOD(ChartParserModule).ParseAndGenerateChartSet(InPath);
					
					if (_SelectedChartSet->IsEmpty())
						return;

					_SelectedChart = _SelectedChartSet->Charts.begin()->second;

					MOD(BeatModule).AssignSnapsToNotesInChart(_SelectedChart);
					MOD(AudioModule).LoadAudio(_SelectedChart->AudioPath);
					MOD(EditModule).SetChart(_SelectedChart);
					MOD(BackgroundModule).LoadBackground(_SelectedChart->BackgroundPath);
					MOD(TimefieldRenderModule).SetKeyAmount(_SelectedChart->KeyAmount);
					MOD(MiniMapModule).Generate(_SelectedChart, MOD(TimefieldRenderModule).GetSkin(), MOD(AudioModule).GetSongLengthMilliSeconds());
				
					_SelectedChart->RegisterOnModifiedCallback([this]()
					{
						//replicate the timeslice method to optimize when "re-generating" 
						MOD(MiniMapModule).Generate(_SelectedChart, MOD(TimefieldRenderModule).GetSkin(), MOD(AudioModule).GetSongLengthMilliSeconds());
					});
				});
			}

			if (ImGui::MenuItem("Save", "CTRL+S"))
			{
				//scream
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}


void Program::InputActions() 
{
	if (MOD(InputModule).IsTogglingPause())
		MOD(AudioModule).TogglePause();

	if(MOD(InputModule).IsCtrlKeyDown() && MOD(InputModule).WasKeyPressed(sf::Keyboard::Key::Z))
		_SelectedChart->Undo();

	if(MOD(InputModule).WasKeyPressed(sf::Keyboard::Key::Num1))
		MOD(EditModule).SetEditMode<SelectEditMode>();

	if(MOD(InputModule).WasKeyPressed(sf::Keyboard::Key::Num2))
		MOD(EditModule).SetEditMode<NoteEditMode>();

	if (MOD(InputModule).IsCtrlKeyDown())
	{
		if (MOD(InputModule).IsScrollingUp())
			ApplyDeltaToZoom(0.1f);

		if (MOD(InputModule).IsScrollingDown())
			ApplyDeltaToZoom(-0.1f);
	}
	else  if (MOD(InputModule).IsAltKeyDown())
	{
		if (MOD(InputModule).IsScrollingUp())
			_CurrentSnap = MOD(BeatModule).GetNextSnap(_CurrentSnap);

		if (MOD(InputModule).IsScrollingDown())
			_CurrentSnap = MOD(BeatModule).GetPreviousSnap(_CurrentSnap);
	}
	else if (MOD(InputModule).IsShiftKeyDown())
	{
		if (MOD(InputModule).IsScrollingUp())
			MOD(AudioModule).ChangeSpeed(0.05f);

		if (MOD(InputModule).IsScrollingDown())
			MOD(AudioModule).ChangeSpeed(-0.05f);
	}
	else
	{
		if (MOD(InputModule).IsScrollingUp())
			MOD(AudioModule).SetTimeMilliSeconds(MOD(BeatModule).GetPreviousBeatLine(MOD(AudioModule).GetTimeMilliSeconds()).TimePoint);

		if (MOD(InputModule).IsScrollingDown())
			MOD(AudioModule).SetTimeMilliSeconds(MOD(BeatModule).GetNextBeatLine(MOD(AudioModule).GetTimeMilliSeconds()).TimePoint);
	}

	if (MOD(InputModule).WasMouseButtonPressed(sf::Mouse::Left))
		MOD(EditModule).OnMouseLeftButtonClicked(MOD(InputModule).IsShiftKeyDown());

	if (MOD(InputModule).WasMouseButtonReleased(sf::Mouse::Left))
		MOD(EditModule).OnMouseLeftButtonReleased();

	if (MOD(InputModule).WasMouseButtonPressed(sf::Mouse::Right))
		MOD(EditModule).OnMouseRightButtonClicked(MOD(InputModule).IsShiftKeyDown());

	if (MOD(InputModule).WasMouseButtonReleased(sf::Mouse::Right))
		MOD(EditModule).OnMouseRightButtonReleased();
}

void Program::ApplyDeltaToZoom(const float InDelta)
{
	_ZoomLevel += InDelta;

	if (_ZoomLevel < 0.25f)
		_ZoomLevel = 0.25f;
	
	if (_ZoomLevel > 2.0f)
		_ZoomLevel = 2.0f;
}

void Program::UpdateCursorData()
{
	//TODO: clean?
	_Cursor.X = (int)ImGui::GetMousePos().x;
	_Cursor.Y = (int)ImGui::GetMousePos().y;
	
	BeatLine snappedBeatLine = MOD(BeatModule).GetCurrentBeatLine(MOD(TimefieldRenderModule).GetTimeFromScreenPoint(_Cursor.Y, MOD(AudioModule).GetTimeMilliSeconds(), _ZoomLevel, true), 1);
	_Cursor.TimeFieldY = MOD(TimefieldRenderModule).GetScreenTimePoint(snappedBeatLine.TimePoint, MOD(AudioModule).GetTimeMilliSeconds(), _ZoomLevel);

	if (_Cursor.TimeFieldY > _WindowMetrics.Height)
	{
		snappedBeatLine = MOD(BeatModule).GetCurrentBeatLine(MOD(TimefieldRenderModule).GetTimeFromScreenPoint(_Cursor.Y, MOD(AudioModule).GetTimeMilliSeconds(), _ZoomLevel, true), 0, false);
		_Cursor.TimeFieldY = MOD(TimefieldRenderModule).GetScreenTimePoint(snappedBeatLine.TimePoint, MOD(AudioModule).GetTimeMilliSeconds(), _ZoomLevel);
	}

	_Cursor.TimePoint = snappedBeatLine.TimePoint;
	_Cursor.UnsnappedTimePoint = MOD(TimefieldRenderModule).GetTimeFromScreenPoint(_Cursor.Y, MOD(AudioModule).GetTimeMilliSeconds(), _ZoomLevel);
	_Cursor.Column = MOD(TimefieldRenderModule).GetColumnFromScreenPoint(_Cursor.X);
	_Cursor.BeatSnap = snappedBeatLine.BeatSnap;

	_Cursor.HoveredNotes.clear();

	MOD(TimefieldRenderModule).GetOverlappedOnScreenNotes(_Cursor.Column, _Cursor.Y, _Cursor.HoveredNotes);

	std::sort(_Cursor.HoveredNotes.begin(), _Cursor.HoveredNotes.end(), [](const Note* lhs, const Note* rhs)  {return lhs->TimePoint < rhs->TimePoint; });
}