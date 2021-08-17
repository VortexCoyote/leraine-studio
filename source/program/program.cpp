#include "program.h"

#include "imgui.h"
#include "../utilities/imgui/addons/imgui_user.h"
#include "../utilities/imgui/std/imgui-stdlib.h"

#include "../structures/chart-metadata.h"
#include "../structures/configuration.h"

namespace
{
	Time WindowTimeBegin;
	Time WindowTimeEnd;

	Cursor EditCursor;

	TimefieldRenderGraph NoteRenderGraph;
	TimefieldRenderGraph PreviewRenderGraph;
	TimefieldRenderGraph WaveformRenderGraph;
	TimefieldRenderGraph DebugRenderGraph;

	Chart* SelectedChart = nullptr;

	float ZoomLevel = 1.0f;
	int CurrentSnap = 2;
	
	ChartMetadata ChartMetadataSetup;

	Configuration Config;

	bool ShouldSetUpMetadata = false;
	bool ShouldSetUpNewChart = false;

	std::string TimeToGo = "0";

	int ZoomIndex = 5;
	std::vector<float> LegalZoomLevels = { 0.25f, 0.4f, 0.5f, 0.6f, 0.75f, 1.0f, 1.25f, 1.4f, 1.5f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f };
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
#include "../modules/notification-module.h"
#include "../modules/shortcut-menu-module.h"
#include "../modules/debug-module.h"

void Program::RegisterModules()
{
	ModuleManager::Register<BackgroundModule>();
	ModuleManager::Register<TimefieldRenderModule>();
	ModuleManager::Register<ImGuiModule>();
	ModuleManager::Register<DialogModule>();
	ModuleManager::Register<PopupModule>();
	ModuleManager::Register<InputModule>();
	ModuleManager::Register<ShortcutMenuModule>();
	ModuleManager::Register<MiniMapModule>();
	ModuleManager::Register<NotificationModule>();
	ModuleManager::Register<ChartParserModule>();
	ModuleManager::Register<AudioModule>();
	ModuleManager::Register<WaveFormModule>();
	ModuleManager::Register<BeatModule>();
	ModuleManager::Register<EditModule>();
	ModuleManager::Register<DebugModule>();
}

void Program::InnerStartUp()
{
	MOD(ImGuiModule).Init(_RenderWindow);
	MOD(NotificationModule).SetStartY(_WindowMetrics.MenuBarHeight + 16);

	if(Config.Load())
	{
		SetConfig(Config);
		PUSH_NOTIFICATION("Config loaded");
	}
	else PUSH_NOTIFICATION("Config file not found. Created a new one");
}

void Program::InnerTick()
{
	 MenuBar();

	if(ShouldSetUpMetadata)
		SetUpMetadata();

	if (!SelectedChart)
		return;

	UpdateCursor();
	MOD(EditModule).SetCursorData(EditCursor);

	WindowTimeBegin = MOD(TimefieldRenderModule).GetWindowTimePointBegin(MOD(AudioModule).GetTimeMilliSeconds(), ZoomLevel);
	WindowTimeEnd = MOD(TimefieldRenderModule).GetWindowTimePointEnd(MOD(AudioModule).GetTimeMilliSeconds(), ZoomLevel);

	if (!ImGui::GetIO().WantCaptureMouse && !ImGui::GetIO().WantTextInput)
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

	if(Config.ShowWaveform)
		MOD(WaveFormModule).RenderWaveForm(WaveformRenderGraph, WindowTimeBegin, WindowTimeEnd, MOD(TimefieldRenderModule).GetTimefieldMetrics().LeftSidePosition + MOD(TimefieldRenderModule).GetTimefieldMetrics().FieldWidthHalf, ZoomLevel, InOutRenderTarget->getView().getSize().y);
		//MOD(WaveFormModule).RenderWaveFormPolygon(InOutRenderTarget, WindowTimeBegin, WindowTimeEnd, MOD(TimefieldRenderModule).GetTimefieldMetrics().LeftSidePosition + MOD(TimefieldRenderModule).GetTimefieldMetrics().FieldWidthHalf, ZoomLevel, InOutRenderTarget->getView().getSize().y);

	MOD(BeatModule).IterateThroughBeatlines([this, &InOutRenderTarget](const BeatLine &InBeatLine)
	{
		MOD(TimefieldRenderModule).RenderBeatLine(InOutRenderTarget, InBeatLine.TimePoint, InBeatLine.BeatSnap, MOD(AudioModule).GetTimeMilliSeconds(), ZoomLevel);
	});

	MOD(DebugModule).RenderTimeSliceBoundaries(DebugRenderGraph, SelectedChart, WindowTimeBegin, WindowTimeEnd);

	MOD(TimefieldRenderModule).RenderTimefieldGraph(InOutRenderTarget, WaveformRenderGraph, MOD(AudioModule).GetTimeMilliSeconds(), ZoomLevel);
	MOD(TimefieldRenderModule).RenderTimefieldGraph(InOutRenderTarget, DebugRenderGraph, MOD(AudioModule).GetTimeMilliSeconds(), ZoomLevel, false);

	MOD(TimefieldRenderModule).RenderReceptors(InOutRenderTarget, CurrentSnap);

	MOD(TimefieldRenderModule).RenderTimefieldGraph(InOutRenderTarget, NoteRenderGraph, MOD(AudioModule).GetTimeMilliSeconds(), ZoomLevel);
	MOD(TimefieldRenderModule).RenderTimefieldGraph(InOutRenderTarget, PreviewRenderGraph, MOD(AudioModule).GetTimeMilliSeconds(), ZoomLevel, false);

	MOD(MiniMapModule).Render(InOutRenderTarget);

	if (MOD(MiniMapModule).ShouldPreview()) // :D ???
		MOD(MiniMapModule).RenderPreview(InOutRenderTarget, MOD(TimefieldRenderModule).GetRenderedTimefieldGraphSegment(MOD(MiniMapModule).GetPreviewRenderGraph(SelectedChart), MOD(MiniMapModule).GetHoveredTime(), ZoomLevel));

	NoteRenderGraph.ClearRenderCommands();
	PreviewRenderGraph.ClearRenderCommands();
	WaveformRenderGraph.ClearRenderCommands();
	DebugRenderGraph.ClearRenderCommands();
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
		if (MOD(ShortcutMenuModule).BeginMenu("File"))
		{
			if (MOD(ShortcutMenuModule).MenuItem("New Chart", sf::Keyboard::Key::LControl, sf::Keyboard::Key::N))
			{
				ChartMetadataSetup = ChartMetadata();
				ShouldSetUpNewChart = true;
				ShouldSetUpMetadata = true;
			}

			if (MOD(ShortcutMenuModule).MenuItem("Open", sf::Keyboard::Key::LControl, sf::Keyboard::Key::O))
			{
				MOD(DialogModule).OpenFileDialog(".osu;.sm", [this](const std::string &InPath) 
				{
					OpenChart(InPath);
				});
			}

			MOD(ShortcutMenuModule).Separator();
			
			if (MOD(ShortcutMenuModule).MenuItem("Edit Metadata", sf::Keyboard::Key::LControl, sf::Keyboard::Key::E) && SelectedChart)
				ShouldSetUpMetadata = true;

			if (MOD(ShortcutMenuModule).MenuItem("Save", sf::Keyboard::Key::LControl, sf::Keyboard::Key::S) && SelectedChart)
				MOD(ChartParserModule).ExportChartSet(SelectedChart);

			MOD(ShortcutMenuModule).Separator();

			for (auto path : Config.RecentFilePaths)
				if (MOD(ShortcutMenuModule).MenuItem(path.c_str(), sf::Keyboard::Unknown, sf::Keyboard::Unknown))
					OpenChart(path);

			MOD(ShortcutMenuModule).EndMenu();
		}

		if (MOD(ShortcutMenuModule).BeginMenu("Edit"))
		{
			if (MOD(ShortcutMenuModule).MenuItem("Undo", sf::Keyboard::Key::LControl, sf::Keyboard::Key::Z) && SelectedChart && SelectedChart->Undo())
				PUSH_NOTIFICATION("Undo");
			
			if (MOD(ShortcutMenuModule).MenuItem("Select All", sf::Keyboard::Key::LControl, sf::Keyboard::Key::A))
				MOD(EditModule).OnSelectAll();

			MOD(ShortcutMenuModule).Separator();

			if (MOD(ShortcutMenuModule).MenuItem("Copy", sf::Keyboard::Key::LControl, sf::Keyboard::Key::C) && SelectedChart)
				MOD(EditModule).OnCopy();

			if (MOD(ShortcutMenuModule).MenuItem("Paste", sf::Keyboard::Key::LControl, sf::Keyboard::Key::V) && SelectedChart)
			 	MOD(EditModule).OnPaste();

			if (MOD(ShortcutMenuModule).MenuItem("Delete", sf::Keyboard::Unknown, sf::Keyboard::Key::Delete) && SelectedChart)
				MOD(EditModule).OnDelete();

			MOD(ShortcutMenuModule).Separator();

			if (MOD(ShortcutMenuModule).MenuItem("Mirror", sf::Keyboard::Key::LControl, sf::Keyboard::Key::H) && SelectedChart)
				MOD(EditModule).OnMirror();

			if (MOD(ShortcutMenuModule).MenuItem("Go To Timepoint", sf::Keyboard::Key::LControl, sf::Keyboard::Key::T) && SelectedChart)
				GoToTimePoint();
				
			MOD(ShortcutMenuModule).EndMenu();
		}

		if (ImGui::BeginMenu("Options"))
		{
			std::string togglePitch = "Toggle Pitch (";
			togglePitch += Config.UsePitch ? "Pitched)" : "Stretched)";

			if(ImGui::MenuItem(togglePitch.c_str()))
			{
				Config.UsePitch = !Config.UsePitch;
				Config.Save();

				MOD(AudioModule).ResetSpeed();
				MOD(AudioModule).UsePitch = Config.UsePitch;

				PUSH_NOTIFICATION("Speed Reset");
			}

			ImGui::Separator();

			if(ImGui::MenuItem("Select Skin"))
			{ 
				MOD(DialogModule).OpenFolderDialog([this](const std::string &InPath) 
				{
					Config.SkinFolderPath = InPath;
					Config.Save();

					if (SelectedChart){
						MOD(TimefieldRenderModule).InitializeResources(SelectedChart->KeyAmount, Config.SkinFolderPath);
					}

					PUSH_NOTIFICATION("Skin Changed");
				}, true);
			}

			ImGui::Separator();

			if(ImGui::Checkbox("Show Column Lines", &Config.ShowColumnLines))
			{
				MOD(TimefieldRenderModule).GetSkin().ShowColumnLines = Config.ShowColumnLines;
				Config.Save();
			}

			if (ImGui::Checkbox("Show Waveform", &Config.ShowWaveform)) Config.Save();

			if (ImGui::Checkbox("Use Auto Timing", &Config.UseAutoTiming))
			{
				EditMode::static_Flags.UseAutoTiming = Config.UseAutoTiming;
				Config.Save();
			}

			if (ImGui::Checkbox("Show Column Heatmap", &Config.ShowColumnHeatmap))
			{
				EditMode::static_Flags.ShowColumnHeatmap = Config.ShowColumnHeatmap;
				Config.Save();
			}

			ImGui::EndMenu();
		}

		if (MOD(ShortcutMenuModule).BeginMenu("Help"))
		{
			if (MOD(ShortcutMenuModule).MenuItem("Shortcuts", sf::Keyboard::Unknown, sf::Keyboard::F1))
				ShowShortCuts();

			MOD(ShortcutMenuModule).EndMenu();
		}
		
		if (ImGui::BeginMenu("Debug"))
		{
			ImGui::Checkbox("Show TimeSlice Boundaries", &MOD(DebugModule).ShowTimeSliceBoundaries);		

			ImGui::EndMenu();
		}


		ImGui::EndMainMenuBar();
	}
}

void Program::SetUpMetadata() 
{
	std::string titleName;

	if (ShouldSetUpNewChart) 
		titleName = "New Chart";
	else
		titleName = "Edit Metadata";

	MOD(PopupModule).OpenPopup(titleName, [this](bool& OutOpen)
	{
			ImGui::Text("Meta Data");
			ImGui::InputText("Artist", &ChartMetadataSetup.Artist);
			ImGui::InputText("Song Title", &ChartMetadataSetup.SongTitle);
			ImGui::InputText("Charter", &ChartMetadataSetup.Charter);
			ImGui::InputText("Difficulty Name", &ChartMetadataSetup.DifficultyName);

			ImGui::NewLine();

			ImGui::Text("Difficulty");
			if (ShouldSetUpNewChart) ImGui::SliderInt("Key Amount", &ChartMetadataSetup.KeyAmount, 4, 10);
			else {
				std::string key = "Key = ";
				key.append(std::to_string(SelectedChart->KeyAmount));
				ImGui::Text(key.c_str());
			}

			ImGui::DragFloat("OD", &ChartMetadataSetup.OD, 0.1f, 1.0f, 10.0f);
			ImGui::DragFloat("HP", &ChartMetadataSetup.HP, 0.1f, 1.0f, 10.0f);

			ImGui::NewLine();

			std::string audioButtonName =  ChartMetadataSetup.AudioPath.string() == "" ? "Pick an audio file" : ChartMetadataSetup.AudioPath.string();
			std::string chartButtonName =  ChartMetadataSetup.ChartFolderPath.string() == "" ? "Pick a chart folder path" : ChartMetadataSetup.ChartFolderPath.string();
			std::string backgroundButtonName =  ChartMetadataSetup.BackgroundPath.string() == "" ? "Pick a background (optional)" : ChartMetadataSetup.BackgroundPath.string();
			
			ImGui::Text("Relevant Paths");
			
			if(ImGui::Button(audioButtonName.c_str()))
			{
				ShouldSetUpMetadata = OutOpen = false;

				MOD(DialogModule).OpenFileDialog(".mp3;.ogg;.wav", [this](const std::string &InPath) 
				{
					ChartMetadataSetup.AudioPath = InPath;
					ShouldSetUpMetadata = true;
				}, true);
			}

			if(ImGui::Button(chartButtonName.c_str()))
			{
				ShouldSetUpMetadata = OutOpen = false;

				MOD(DialogModule).OpenFolderDialog([this](const std::string &InPath) 
				{
					ChartMetadataSetup.ChartFolderPath = InPath;
					ShouldSetUpMetadata = true;
				}, true);
			}

			if(ImGui::Button(backgroundButtonName.c_str()))
			{
				ShouldSetUpMetadata = OutOpen = false;

				MOD(DialogModule).OpenFileDialog(".png;.jpg", [this](const std::string &InPath) 
				{
					ChartMetadataSetup.BackgroundPath = InPath;
					ShouldSetUpMetadata = true;
				}, true);
			}

			ImGui::NewLine();

			if(ShouldSetUpNewChart)
			{
				if(ImGui::Button("Create"))
				{
					OpenChart(MOD(ChartParserModule).CreateNewChart(ChartMetadataSetup));

					ShouldSetUpMetadata = ShouldSetUpNewChart = OutOpen = false;
				}
			}

			else
			{
				if(ImGui::Button("Save"))
				{
					OpenChart(MOD(ChartParserModule).SetChartMetadata(SelectedChart, ChartMetadataSetup));

					ShouldSetUpMetadata = ShouldSetUpNewChart = OutOpen = false;
				}
			}

			ImGui::SameLine();
			
			if(ImGui::Button("Close"))
			{
				if (ShouldSetUpNewChart) ChartMetadataSetup = ChartMetadata();
				else ChartMetadataSetup = MOD(ChartParserModule).GetChartMetadata(SelectedChart);
				ShouldSetUpMetadata = ShouldSetUpNewChart = OutOpen = false;
			}
	});
}

void Program::ShowShortCuts() 
{
	MOD(PopupModule).OpenPopup("New Chart", [this](bool& OutOpen)
	{
		MOD(ShortcutMenuModule).ShowCheatSheet();

		if(ImGui::Button("close"))
			OutOpen = false;
	});
}

void Program::GoToTimePoint() 
{
	MOD(PopupModule).OpenPopup("Go To Timepoint", [this](bool& OutOpen)
	{
		ImGui::InputText("Timepoint (MS)", &TimeToGo);

		if(ImGui::Button("Go") || MOD(InputModule).WasKeyPressed(sf::Keyboard::Key::Enter))
		{
			MOD(AudioModule).SetTimeMilliSeconds(std::stoi(TimeToGo));

			OutOpen = false;
		}

		ImGui::SameLine();

		if(ImGui::Button("Cancel") || MOD(InputModule).WasKeyPressed(sf::Keyboard::Key::Escape))
			OutOpen = false;
	});
}

void Program::InputActions()
{
	MOD(EditModule).SetShiftKeyState(MOD(InputModule).IsShiftKeyDown());

	if (MOD(InputModule).IsTogglingPause())
		MOD(AudioModule).TogglePause();

	if (MOD(InputModule).WasKeyPressed(sf::Keyboard::Key::Num1))
	{
		MOD(EditModule).SetEditMode<SelectEditMode>();
		PUSH_NOTIFICATION("Select Edit Mode");
	}

	if (MOD(InputModule).WasKeyPressed(sf::Keyboard::Key::Num2))
	{
		MOD(EditModule).SetEditMode<NoteEditMode>();
		PUSH_NOTIFICATION("Note Edit Mode");
	}

	if (MOD(InputModule).WasKeyPressed(sf::Keyboard::Key::Num3))
	{
		MOD(EditModule).SetEditMode<BpmEditMode>();
		PUSH_NOTIFICATION("Bpm Edit Mode");
	}

	if (MOD(InputModule).IsCtrlKeyDown())
	{
		if (MOD(InputModule).IsScrollingUp())
			return ApplyDeltaToZoom(1.0f);

		if (MOD(InputModule).IsScrollingDown())
			return ApplyDeltaToZoom(-1.0f);
	}

	if (MOD(InputModule).IsAltKeyDown())
	{
		if (MOD(InputModule).IsScrollingUp())
			return void(CurrentSnap = MOD(BeatModule).GetNextSnap(CurrentSnap)), PUSH_NOTIFICATION_LIFETIME(0.5f, "Snap %d", CurrentSnap);

		if (MOD(InputModule).IsScrollingDown())
			return void(CurrentSnap = MOD(BeatModule).GetPreviousSnap(CurrentSnap)), PUSH_NOTIFICATION_LIFETIME(0.5f, "Snap %d", CurrentSnap);
	}

	if (MOD(InputModule).IsShiftKeyDown())
	{
		if (MOD(InputModule).IsScrollingUp() || MOD(InputModule).IsRightKeyPressed())
			return MOD(AudioModule).ChangeSpeed(0.05f), PUSH_NOTIFICATION_LIFETIME(0.5f, "Speed %.2fx", MOD(AudioModule).GetPlaybackSpeed());

		if (MOD(InputModule).IsScrollingDown() || MOD(InputModule).IsLeftKeyPressed())
			return MOD(AudioModule).ChangeSpeed(-0.05f), PUSH_NOTIFICATION_LIFETIME(0.5f, "Speed %.2fx", MOD(AudioModule).GetPlaybackSpeed());
	}

	if (MOD(InputModule).IsScrollingUp() || MOD(InputModule).IsDownKeyPressed())
		return MOD(AudioModule).SetTimeMilliSeconds(MOD(BeatModule).GetPreviousBeatLine(MOD(AudioModule).GetTimeMilliSeconds()).TimePoint);

	if (MOD(InputModule).IsScrollingDown() || MOD(InputModule).IsUpKeyPressed())
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

void Program::OpenChart(const std::string InPath) 
{
	SelectedChart = MOD(ChartParserModule).ParseAndGenerateChartSet(InPath);
	
	if (!SelectedChart)
	{
		PUSH_NOTIFICATION("File not found! It might have been deleted?");
		Config.DeleteRecentFile(InPath);
		Config.Save();
		return;
	}

	Config.RegisterRecentFile(InPath);
	Config.Save();

	MOD(BeatModule).AssignNotesToSnapsInChart(SelectedChart);
	MOD(AudioModule).LoadAudio(SelectedChart->AudioPath);
	MOD(EditModule).SetChart(SelectedChart);
	MOD(BackgroundModule).LoadBackground(SelectedChart->BackgroundPath);
	MOD(TimefieldRenderModule).InitializeResources(SelectedChart->KeyAmount, Config.SkinFolderPath);
	MOD(MiniMapModule).Generate(SelectedChart, MOD(TimefieldRenderModule).GetSkin(), MOD(AudioModule).GetSongLengthMilliSeconds());
	MOD(WaveFormModule).SetWaveFormData(MOD(AudioModule).GenerateAndGetWaveformData(SelectedChart->AudioPath), MOD(AudioModule).GetSongLengthMilliSeconds());
	ChartMetadataSetup = MOD(ChartParserModule).GetChartMetadata(SelectedChart);

	SelectedChart->RegisterOnModifiedCallback([this](TimeSlice &InTimeSlice) 
	{
		//TODO: replicate the timeslice method to optimize when "re-generating"
		MOD(BeatModule).AssignNotesToSnapsInTimeSlice(SelectedChart, InTimeSlice);
		MOD(MiniMapModule).GeneratePortion(InTimeSlice, MOD(TimefieldRenderModule).GetSkin());
	});
}

void Program::ApplyDeltaToZoom(const float InDelta)
{
	if(InDelta > 0)
		ZoomIndex++;
	else
		ZoomIndex--;

	if(ZoomIndex > int(LegalZoomLevels.size() - 1))
		ZoomIndex =	LegalZoomLevels.size() - 1;

	if(ZoomIndex < 0)
		ZoomIndex = 0;

	ZoomLevel = LegalZoomLevels[ZoomIndex];
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
	EditCursor.CursorColumn = MOD(TimefieldRenderModule).GetColumnFromScreenPoint(EditCursor.X);
	EditCursor.BeatSnap = snappedBeatLine.BeatSnap;

	if (EditCursor.X < timefieldMetrics.LeftSidePosition)
		EditCursor.TimefieldSide = Cursor::FieldPosition::Left;
	else if (EditCursor.X > timefieldMetrics.LeftSidePosition + timefieldMetrics.FieldWidth)
		EditCursor.TimefieldSide = Cursor::FieldPosition::Right;
	else
		EditCursor.TimefieldSide = Cursor::FieldPosition::Middle;

	EditCursor.HoveredNotes.clear();

	MOD(TimefieldRenderModule).GetOverlappedOnScreenNotes(EditCursor.CursorColumn, EditCursor.Y, EditCursor.HoveredNotes);

	std::sort(EditCursor.HoveredNotes.begin(), EditCursor.HoveredNotes.end(), [](const Note *lhs, const Note *rhs) { return lhs->TimePoint < rhs->TimePoint; });
}

void Program::SetConfig(Configuration config)
{
	MOD(AudioModule).UsePitch = Config.UsePitch;
	MOD(TimefieldRenderModule).GetSkin().ShowColumnLines = Config.ShowColumnLines;
	EditMode::static_Flags.UseAutoTiming = Config.UseAutoTiming;
	EditMode::static_Flags.ShowColumnHeatmap = Config.ShowColumnHeatmap;
}