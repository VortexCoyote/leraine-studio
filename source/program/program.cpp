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

//file exclusive globals
Time WindowTimeBegin;
Time WindowTimeEnd;

Cursor EditCursor;


void Program::RegisterModules()
{
	ModuleManager::Register<BackgroundModule>();
	ModuleManager::Register<TimefieldRenderModule>();
	ModuleManager::Register<ImGuiModule>();
	ModuleManager::Register<DialogModule>();
	ModuleManager::Register<InputModule>();
	ModuleManager::Register<MiniMapModule>();
	ModuleManager::Register<ChartParserModule>();
	ModuleManager::Register<AudioModule>();
	ModuleManager::Register<BeatModule>();
	ModuleManager::Register<EditModule>();
}

void Program::InnerStartUp()
{
	MOD(ImGuiModule).Init(_RenderWindow);
}

void Program::InnerTick()
{
	MenuBar();

	if (!_SelectedChart)
		return;

	UpdateCursor();
	MOD(EditModule).SetCursorData(EditCursor);
	
	WindowTimeBegin = MOD(TimefieldRenderModule).GetWindowTimePointBegin(MOD(AudioModule).GetTimeMilliSeconds(), _ZoomLevel);
	WindowTimeEnd   = MOD(TimefieldRenderModule).GetWindowTimePointEnd(MOD(AudioModule).GetTimeMilliSeconds(), _ZoomLevel);

	InputActions();

	MOD(TimefieldRenderModule).UpdateMetrics(_WindowMetrics);
	MOD(BeatModule).GenerateTimeRangeBeatLines(WindowTimeBegin, WindowTimeEnd, _SelectedChart, _CurrentSnap);

	_SelectedChart->IterateNotesInTimeRange(WindowTimeBegin - TIMESLICE_LENGTH, WindowTimeEnd, [this](Note& InNote, const Column InColumn)
	{
		_ChartRenderGraph.SubmitNoteRenderCommand(InNote, InColumn);
	});

	MOD(EditModule).SubmitToRenderGraph(_PreviewRenderGraph);
}

void Program::InnerRender(sf::RenderTarget* const InOutRenderTarget)
{
	if (!_SelectedChart)
		return;

	MOD(BeatModule).IterateThroughBeatlines([this, &InOutRenderTarget](const BeatLine& InBeatLine)
	{
		MOD(TimefieldRenderModule).RenderBeatLine(InOutRenderTarget, InBeatLine.TimePoint, InBeatLine.BeatSnap, MOD(AudioModule).GetTimeMilliSeconds(), _ZoomLevel);
	});

	MOD(TimefieldRenderModule).RenderTimefieldGraph(InOutRenderTarget ,_ChartRenderGraph, MOD(AudioModule).GetTimeMilliSeconds(), _ZoomLevel);
	MOD(TimefieldRenderModule).RenderTimefieldGraph(InOutRenderTarget, _PreviewRenderGraph, MOD(AudioModule).GetTimeMilliSeconds(), _ZoomLevel, false);
	
	MOD(MiniMapModule).Render(InOutRenderTarget);

	if(MOD(MiniMapModule).ShouldPreview()) // :D ???
		MOD(MiniMapModule).RenderPreview(InOutRenderTarget, MOD(TimefieldRenderModule).GetRenderedTimefieldGraphSegment(MOD(MiniMapModule).GetPreviewRenderGraph(_SelectedChart), MOD(MiniMapModule).GetHoveredTime(), _ZoomLevel));

	_ChartRenderGraph.ClearRenderCommands();
	_PreviewRenderGraph.ClearRenderCommands();
}

void Program::InnerShutDown()
{
	
}

//************************************************************************************************************************************************************************************

void Program::MenuBar()
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
						//TODO: replicate the timeslice method to optimize when "re-generating" 
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
	//TODO: set up input action priorities (maybe make some sort of dependency graph module?)

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

		return;
	}
	
	if (MOD(InputModule).IsAltKeyDown())
	{
		if (MOD(InputModule).IsScrollingUp())
			_CurrentSnap = MOD(BeatModule).GetNextSnap(_CurrentSnap);

		if (MOD(InputModule).IsScrollingDown())
			_CurrentSnap = MOD(BeatModule).GetPreviousSnap(_CurrentSnap);

		return;
	}
	
	if (MOD(InputModule).IsShiftKeyDown())
	{
		if (MOD(InputModule).IsScrollingUp())
			MOD(AudioModule).ChangeSpeed(0.05f);

		if (MOD(InputModule).IsScrollingDown())
			MOD(AudioModule).ChangeSpeed(-0.05f);

		return;
	}
	
	if (MOD(InputModule).IsScrollingUp())
		return MOD(AudioModule).SetTimeMilliSeconds(MOD(BeatModule).GetPreviousBeatLine(MOD(AudioModule).GetTimeMilliSeconds()).TimePoint);

	if (MOD(InputModule).IsScrollingDown())
		return MOD(AudioModule).SetTimeMilliSeconds(MOD(BeatModule).GetNextBeatLine(MOD(AudioModule).GetTimeMilliSeconds()).TimePoint);
	
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	if(MOD(MiniMapModule).IsDragging())
	{
		if (MOD(InputModule).WasMouseButtonReleased(sf::Mouse::Left))
			return MOD(MiniMapModule).EndDragging();		
			
		MOD(AudioModule).SetTimeMilliSeconds((MOD(MiniMapModule).GetHoveredTime()));
	}
	
	if(MOD(MiniMapModule).IsHoveringTimeline(32, _WindowMetrics.MenuBarHeight, _WindowMetrics.Height - _WindowMetrics.MenuBarHeight, 16, MOD(AudioModule).GetTimeMilliSeconds(), WindowTimeBegin, WindowTimeEnd, EditCursor))
	{
		if(MOD(MiniMapModule).IsPossibleToDrag())
		{
			if (MOD(InputModule).WasMouseButtonPressed(sf::Mouse::Left))	
				return MOD(MiniMapModule).StartDragging();
		}
		else if(!MOD(MiniMapModule).IsDragging())
		{
			if (MOD(InputModule).WasMouseButtonPressed(sf::Mouse::Left))		
				return MOD(AudioModule).SetTimeMilliSeconds((MOD(MiniMapModule).GetHoveredTime()));
		}
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

void Program::UpdateCursor()
{
	//TODO: clean?
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	const auto& timefieldMetrics = MOD(TimefieldRenderModule).GetTimefieldMetrics();

	EditCursor.X = (int)ImGui::GetMousePos().x;
	EditCursor.Y = (int)ImGui::GetMousePos().y;
	
	BeatLine snappedBeatLine = MOD(BeatModule).GetCurrentBeatLine(MOD(TimefieldRenderModule).GetTimeFromScreenPoint(EditCursor.Y, MOD(AudioModule).GetTimeMilliSeconds(), _ZoomLevel, true), 1);
	EditCursor.TimeFieldY = MOD(TimefieldRenderModule).GetScreenTimePoint(snappedBeatLine.TimePoint, MOD(AudioModule).GetTimeMilliSeconds(), _ZoomLevel);

	if (EditCursor.TimeFieldY > _WindowMetrics.Height)
	{
		snappedBeatLine = MOD(BeatModule).GetCurrentBeatLine(MOD(TimefieldRenderModule).GetTimeFromScreenPoint(EditCursor.Y, MOD(AudioModule).GetTimeMilliSeconds(), _ZoomLevel, true), 0, false);
		EditCursor.TimeFieldY = MOD(TimefieldRenderModule).GetScreenTimePoint(snappedBeatLine.TimePoint, MOD(AudioModule).GetTimeMilliSeconds(), _ZoomLevel);
	}

	EditCursor.TimePoint = snappedBeatLine.TimePoint;
	EditCursor.UnsnappedTimePoint = MOD(TimefieldRenderModule).GetTimeFromScreenPoint(EditCursor.Y, MOD(AudioModule).GetTimeMilliSeconds(), _ZoomLevel);
	EditCursor.Column = MOD(TimefieldRenderModule).GetColumnFromScreenPoint(EditCursor.X);
	EditCursor.BeatSnap = snappedBeatLine.BeatSnap;
 
	if(EditCursor.X < timefieldMetrics.LeftSidePosition)
		EditCursor.TimefieldSide = Cursor::FieldPosition::Left;
	else if(EditCursor.X > timefieldMetrics.LeftSidePosition + timefieldMetrics.FieldWidth)
		EditCursor.TimefieldSide = Cursor::FieldPosition::Right;
	else
		EditCursor.TimefieldSide = Cursor::FieldPosition::Middle;

	EditCursor.HoveredNotes.clear();

	MOD(TimefieldRenderModule).GetOverlappedOnScreenNotes(EditCursor.Column, EditCursor.Y, EditCursor.HoveredNotes);

	std::sort(EditCursor.HoveredNotes.begin(), EditCursor.HoveredNotes.end(), [](const Note* lhs, const Note* rhs)  {return lhs->TimePoint < rhs->TimePoint; });
}