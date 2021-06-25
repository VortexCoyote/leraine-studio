#include "program.h"

#include "imgui.h"
#include "../utilities/imgui/addons/imgui_user.h"

namespace
{
	Time WindowTimeBegin;
	Time WindowTimeEnd;

	Cursor EditCursor;

	TimefieldRenderGraph NoteRenderGraph;
	TimefieldRenderGraph PreviewRenderGraph;
	TimefieldRenderGraph WaveformRenderGraph;

	Chart* SelectedChart = nullptr;

	float ZoomLevel = 1.0f;
	int CurrentSnap = 2;
	
	bool DebugShowTimeSliceBoundaries = false;
}

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
#include "../modules/waveform-module.h"
#include "../modules/popup-module.h"

void Program::RegisterModules()
{
	ModuleManager::Register<BackgroundModule>();
	ModuleManager::Register<TimefieldRenderModule>();
	ModuleManager::Register<ImGuiModule>();
	ModuleManager::Register<DialogModule>();
	ModuleManager::Register<PopupModule>();
	ModuleManager::Register<InputModule>();
	ModuleManager::Register<MiniMapModule>();
	ModuleManager::Register<ChartParserModule>();
	ModuleManager::Register<AudioModule>();
	ModuleManager::Register<WaveFormModule>();
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

	if (!SelectedChart)
		return;

	UpdateCursor();
	MOD(EditModule).SetCursorData(EditCursor);

	WindowTimeBegin = MOD(TimefieldRenderModule).GetWindowTimePointBegin(MOD(AudioModule).GetTimeMilliSeconds(), ZoomLevel);
	WindowTimeEnd = MOD(TimefieldRenderModule).GetWindowTimePointEnd(MOD(AudioModule).GetTimeMilliSeconds(), ZoomLevel);

	if (!ImGui::GetIO().WantCaptureMouse)
		InputActions();

	MOD(TimefieldRenderModule).UpdateMetrics(_WindowMetrics);
	MOD(BeatModule).GenerateTimeRangeBeatLines(WindowTimeBegin, WindowTimeEnd, SelectedChart, CurrentSnap);

	SelectedChart->IterateNotesInTimeRange(WindowTimeBegin - TIMESLICE_LENGTH, WindowTimeEnd, [this](Note &InNote, const Column InColumn) {
		sf::Int8 alpha = 255;

		if (MOD(EditModule).IsEditModeActive<BpmEditMode>())
			alpha = 128;

		NoteRenderGraph.SubmitNoteRenderCommand(InNote, InColumn, alpha);
	});

	MOD(EditModule).SubmitToRenderGraph(PreviewRenderGraph, WindowTimeBegin, WindowTimeEnd);
}

void Program::InnerRender(sf::RenderTarget *const InOutRenderTarget)
{
	if (!SelectedChart)
		return;

	MOD(WaveFormModule).RenderWaveForm(WaveformRenderGraph, WindowTimeBegin, WindowTimeEnd, MOD(TimefieldRenderModule).GetTimefieldMetrics().LeftSidePosition + MOD(TimefieldRenderModule).GetTimefieldMetrics().FieldWidthHalf, ZoomLevel, InOutRenderTarget->getView().getSize().y);

	MOD(BeatModule).IterateThroughBeatlines([this, &InOutRenderTarget](const BeatLine &InBeatLine)
	{
		MOD(TimefieldRenderModule).RenderBeatLine(InOutRenderTarget, InBeatLine.TimePoint, InBeatLine.BeatSnap, MOD(AudioModule).GetTimeMilliSeconds(), ZoomLevel);
	});

	MOD(TimefieldRenderModule).RenderTimefieldGraph(InOutRenderTarget, WaveformRenderGraph, MOD(AudioModule).GetTimeMilliSeconds(), ZoomLevel);

	MOD(TimefieldRenderModule).RenderReceptors(InOutRenderTarget, CurrentSnap);

	MOD(TimefieldRenderModule).RenderTimefieldGraph(InOutRenderTarget, NoteRenderGraph, MOD(AudioModule).GetTimeMilliSeconds(), ZoomLevel);
	MOD(TimefieldRenderModule).RenderTimefieldGraph(InOutRenderTarget, PreviewRenderGraph, MOD(AudioModule).GetTimeMilliSeconds(), ZoomLevel, false);

	if(DebugShowTimeSliceBoundaries)
	{
		SelectedChart->IterateTimeSlicesInTimeRange(WindowTimeBegin, WindowTimeEnd, [this, InOutRenderTarget](TimeSlice InTimeSlice)
		{
			auto& timeFieldMetrics = MOD(TimefieldRenderModule).GetTimefieldMetrics();

			sf::RectangleShape line(sf::Vector2f(timeFieldMetrics.FieldWidth, 6));
			line.setPosition(timeFieldMetrics.LeftSidePosition, MOD(TimefieldRenderModule).GetScreenPointFromTime(InTimeSlice.TimePoint, MOD(AudioModule).GetTimeMilliSeconds(), ZoomLevel));
	
			line.setFillColor(sf::Color(255, 0, 255, 255));

			InOutRenderTarget->draw(line);

			ImGuiWindowFlags flags = ImGuiWindowFlags_None;
			flags |= ImGuiWindowFlags_NoTitleBar;
			flags |= ImGuiWindowFlags_NoResize;
			flags |= ImGuiWindowFlags_AlwaysAutoResize;

			ImGui::SetNextWindowPos({line.getPosition().x + timeFieldMetrics.FieldWidth + 6.f, line.getPosition().y});
			ImGui::Begin(std::to_string(InTimeSlice.TimePoint).c_str(), &DebugShowTimeSliceBoundaries, flags);			
			ImGui::Text(std::to_string(InTimeSlice.TimePoint).c_str());
			ImGui::End();
		});
	}

	MOD(MiniMapModule).Render(InOutRenderTarget);

	if (MOD(MiniMapModule).ShouldPreview()) // :D ???
		MOD(MiniMapModule).RenderPreview(InOutRenderTarget, MOD(TimefieldRenderModule).GetRenderedTimefieldGraphSegment(MOD(MiniMapModule).GetPreviewRenderGraph(SelectedChart), MOD(MiniMapModule).GetHoveredTime(), ZoomLevel));

	NoteRenderGraph.ClearRenderCommands();
	PreviewRenderGraph.ClearRenderCommands();
	WaveformRenderGraph.ClearRenderCommands();
}

void Program::InnerShutDown()
{
	delete SelectedChart;
}

//************************************************************************************************************************************************************************************

void Program::MenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New File", "CTRL+N"))
			{
				MOD(PopupModule).OpenPopup("New File", [this](bool& OutOpen)
				{
					if(ImGui::Button("Close"))
						OutOpen = false;
					
					if(ImGui::Button("Choose Audio File"))
						MOD(DialogModule).OpenFileDialog(".osu", [this](const std::string &InPath) 
						{
							
						});
				});
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Open", "CTRL+O"))
			{
				MOD(DialogModule).OpenFileDialog(".osu", [this](const std::string &InPath) {
					SelectedChart = MOD(ChartParserModule).ParseAndGenerateChartSet(InPath);

					MOD(BeatModule).AssignNotesToSnapsInChart(SelectedChart);
					MOD(AudioModule).LoadAudio(SelectedChart->AudioPath);
					MOD(EditModule).SetChart(SelectedChart);
					MOD(BackgroundModule).LoadBackground(SelectedChart->BackgroundPath);
					MOD(TimefieldRenderModule).SetKeyAmount(SelectedChart->KeyAmount);
					MOD(MiniMapModule).Generate(SelectedChart, MOD(TimefieldRenderModule).GetSkin(), MOD(AudioModule).GetSongLengthMilliSeconds());
					MOD(WaveFormModule).SetWaveFormData(MOD(AudioModule).GenerateAndGetWaveformData(SelectedChart->AudioPath), MOD(AudioModule).GetSongLengthMilliSeconds());

					SelectedChart->RegisterOnModifiedCallback([this](TimeSlice &InTimeSlice) {
						//TODO: replicate the timeslice method to optimize when "re-generating"
						//MOD(MiniMapModule).Generate(SelectedChart, MOD(TimefieldRenderModule).GetSkin(), MOD(AudioModule).GetSongLengthMilliSeconds());

						MOD(BeatModule).AssignNotesToSnapsInTimeSlice(SelectedChart, InTimeSlice);
					});
				});
			}

			if (ImGui::MenuItem("Save", "CTRL+S"))
			{
				MOD(ChartParserModule).ExportChartSet(SelectedChart, MOD(AudioModule).GetSongLengthMilliSeconds());
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Export", "CTRL+E"))
			{
				
			}

			if (ImGui::MenuItem("Open in osu!", "F5"))
			{

			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z"));

			ImGui::Separator();

			if (ImGui::MenuItem("Copy", "CTRL+C"));

			if (ImGui::MenuItem("Paste", "CTRL+V"));

			if (ImGui::MenuItem("Delete", "DELETE"));

			ImGui::Separator();

			if (ImGui::MenuItem("Mirror", "CTRL+H"));

			if (ImGui::MenuItem("Goto Timepoint", "CTRL+T"));
				
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Options"))
		{
			ImGui::Checkbox("Use Pitched Rate", &EditMode::static_Flags.UseAutoTiming);

			ImGui::Checkbox("Show Column Lines", &EditMode::static_Flags.UseAutoTiming);

			ImGui::Checkbox("Use Auto Timing", &EditMode::static_Flags.UseAutoTiming);

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("Shortcuts", "F1"));

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Debug"))
		{
			ImGui::Checkbox("Show TimeSlice Boundaries", &DebugShowTimeSliceBoundaries);		

			ImGui::EndMenu();
		}


		ImGui::EndMainMenuBar();
	}
}

void Program::InputActions()
{
	if (MOD(InputModule).IsTogglingPause())
		MOD(AudioModule).TogglePause();

	if (MOD(InputModule).IsCtrlKeyDown() && MOD(InputModule).WasKeyPressed(sf::Keyboard::Key::Z))
		SelectedChart->Undo();

	if (MOD(InputModule).WasKeyPressed(sf::Keyboard::Key::Num1))
		MOD(EditModule).SetEditMode<SelectEditMode>();

	if (MOD(InputModule).WasKeyPressed(sf::Keyboard::Key::Num2))
		MOD(EditModule).SetEditMode<NoteEditMode>();

	if (MOD(InputModule).WasKeyPressed(sf::Keyboard::Key::Num3))
		MOD(EditModule).SetEditMode<BpmEditMode>();

	if (MOD(InputModule).IsCtrlKeyDown())
	{
		if (MOD(InputModule).IsScrollingUp())
			return ApplyDeltaToZoom(0.1f);

		if (MOD(InputModule).IsScrollingDown())
			return ApplyDeltaToZoom(-0.1f);

		if (MOD(InputModule).WasKeyPressed(sf::Keyboard::Key::C))
			return void(MOD(EditModule).OnCopy());

		if (MOD(InputModule).WasKeyPressed(sf::Keyboard::Key::V))
			return void(MOD(EditModule).OnPaste());
	}

	if (MOD(InputModule).IsAltKeyDown())
	{
		if (MOD(InputModule).IsScrollingUp())
			return void(CurrentSnap = MOD(BeatModule).GetNextSnap(CurrentSnap));

		if (MOD(InputModule).IsScrollingDown())
			return void(CurrentSnap = MOD(BeatModule).GetPreviousSnap(CurrentSnap));
	}

	if (MOD(InputModule).IsShiftKeyDown())
	{
		if (MOD(InputModule).IsScrollingUp())
			return MOD(AudioModule).ChangeSpeed(0.05f);

		if (MOD(InputModule).IsScrollingDown())
			return MOD(AudioModule).ChangeSpeed(-0.05f);
	}

	if (MOD(InputModule).IsScrollingUp())
		return MOD(AudioModule).SetTimeMilliSeconds(MOD(BeatModule).GetPreviousBeatLine(MOD(AudioModule).GetTimeMilliSeconds()).TimePoint);

	if (MOD(InputModule).IsScrollingDown())
		return MOD(AudioModule).SetTimeMilliSeconds(MOD(BeatModule).GetNextBeatLine(MOD(AudioModule).GetTimeMilliSeconds()).TimePoint);

	if (ImGui::GetIO().WantCaptureMouse)
		return;

	if (MOD(MiniMapModule).IsDragging())
	{
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			return MOD(MiniMapModule).EndDragging();

		MOD(AudioModule).SetTimeMilliSeconds((MOD(MiniMapModule).GetHoveredTime()));
	}

	if (MOD(MiniMapModule).IsHoveringTimeline(32, _WindowMetrics.MenuBarHeight, _WindowMetrics.Height - _WindowMetrics.MenuBarHeight, 16, MOD(AudioModule).GetTimeMilliSeconds(), WindowTimeBegin, WindowTimeEnd, EditCursor))
	{
		if (MOD(MiniMapModule).IsPossibleToDrag())
		{
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				return MOD(MiniMapModule).StartDragging();
		}
		else if (!MOD(MiniMapModule).IsDragging())
		{
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				return MOD(AudioModule).SetTimeMilliSeconds((MOD(MiniMapModule).GetHoveredTime()));
		}

		return;
	}

	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		return void(MOD(EditModule).OnMouseLeftButtonReleased());

	if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
		return void(MOD(EditModule).OnMouseRightButtonReleased());

	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		return void(MOD(EditModule).OnMouseLeftButtonClicked(MOD(InputModule).IsShiftKeyDown()));

	if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		return void(MOD(EditModule).OnMouseRightButtonClicked(MOD(InputModule).IsShiftKeyDown()));
}

void Program::ApplyDeltaToZoom(const float InDelta)
{
	ZoomLevel += InDelta;

	if (ZoomLevel < 0.25f)
		ZoomLevel = 0.25f;

	if (ZoomLevel > 2.0f)
		ZoomLevel = 2.0f;
}

void Program::UpdateCursor()
{
	//TODO: module? get dependencies and throw it into a module
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	const auto &timefieldMetrics = MOD(TimefieldRenderModule).GetTimefieldMetrics();

	EditCursor.X = (int)ImGui::GetMousePos().x;
	EditCursor.Y = (int)ImGui::GetMousePos().y;

	BeatLine snappedBeatLine = MOD(BeatModule).GetCurrentBeatLine(MOD(TimefieldRenderModule).GetTimeFromScreenPoint(EditCursor.Y, MOD(AudioModule).GetTimeMilliSeconds(), ZoomLevel, true), 1);
	EditCursor.TimeFieldY = MOD(TimefieldRenderModule).GetScreenPointFromTime(snappedBeatLine.TimePoint, MOD(AudioModule).GetTimeMilliSeconds(), ZoomLevel);

	if (EditCursor.TimeFieldY > _WindowMetrics.Height)
	{
		snappedBeatLine = MOD(BeatModule).GetCurrentBeatLine(MOD(TimefieldRenderModule).GetTimeFromScreenPoint(EditCursor.Y, MOD(AudioModule).GetTimeMilliSeconds(), ZoomLevel, true), 0, false);
		EditCursor.TimeFieldY = MOD(TimefieldRenderModule).GetScreenPointFromTime(snappedBeatLine.TimePoint, MOD(AudioModule).GetTimeMilliSeconds(), ZoomLevel);
	}

	EditCursor.TimePoint = snappedBeatLine.TimePoint;
	EditCursor.UnsnappedTimePoint = MOD(TimefieldRenderModule).GetTimeFromScreenPoint(EditCursor.Y, MOD(AudioModule).GetTimeMilliSeconds(), ZoomLevel);
	EditCursor.Column = MOD(TimefieldRenderModule).GetColumnFromScreenPoint(EditCursor.X);
	EditCursor.BeatSnap = snappedBeatLine.BeatSnap;

	if (EditCursor.X < timefieldMetrics.LeftSidePosition)
		EditCursor.TimefieldSide = Cursor::FieldPosition::Left;
	else if (EditCursor.X > timefieldMetrics.LeftSidePosition + timefieldMetrics.FieldWidth)
		EditCursor.TimefieldSide = Cursor::FieldPosition::Right;
	else
		EditCursor.TimefieldSide = Cursor::FieldPosition::Middle;

	EditCursor.HoveredNotes.clear();

	MOD(TimefieldRenderModule).GetOverlappedOnScreenNotes(EditCursor.Column, EditCursor.Y, EditCursor.HoveredNotes);

	std::sort(EditCursor.HoveredNotes.begin(), EditCursor.HoveredNotes.end(), [](const Note *lhs, const Note *rhs) { return lhs->TimePoint < rhs->TimePoint; });
}