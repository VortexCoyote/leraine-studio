#include "program.h"

#include "imgui.h"
#include "../utilities/imgui/addons/imgui_user.h"
#include "../utilities/imgui/std/imgui-stdlib.h"

#include "../structures/chart-metadata.h"

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
	
	ChartMetadata SetUpChartMetadata = ChartMetadata();

	bool ShouldSetUpMetadata = false;
	bool ShouldSetUpNewChart = false;
	bool DebugShowTimeSliceBoundaries = false;
	bool ShowWaveform = true;

	std::string TimeToGo = "0";
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

void Program::RegisterModules()
{
	ModuleManager::Register<BackgroundModule>();
	ModuleManager::Register<TimefieldRenderModule>();
	ModuleManager::Register<ImGuiModule>();
	ModuleManager::Register<DialogModule>();
	ModuleManager::Register<PopupModule>();
	ModuleManager::Register<InputModule>();
	ModuleManager::Register<MiniMapModule>();
	ModuleManager::Register<NotificationModule>();
	ModuleManager::Register<ChartParserModule>();
	ModuleManager::Register<AudioModule>();
	ModuleManager::Register<WaveFormModule>();
	ModuleManager::Register<BeatModule>();
	ModuleManager::Register<EditModule>();
}

void Program::InnerStartUp()
{
	MOD(ImGuiModule).Init(_RenderWindow);
	MOD(NotificationModule).SetStartY(_WindowMetrics.MenuBarHeight + 16);
}

void Program::InnerTick()
{
	 MenuBar();
	 GlobalInputActions();

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

	if(ShowWaveform)
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
			if (ImGui::MenuItem("New Chart", "CTRL+N"))
			{
				ShouldSetUpNewChart = true;
				ShouldSetUpMetadata = true;
			}

			if (ImGui::MenuItem("Open", "CTRL+O"))
			{
				MOD(DialogModule).OpenFileDialog(".osu", [this](const std::string &InPath) 
				{
					OpenChart(InPath);
				});
			}

			ImGui::Separator();
			
			if (ImGui::MenuItem("Edit Metadata") && SelectedChart)
				ShouldSetUpMetadata = true;

			if (ImGui::MenuItem("Save", "CTRL+S") && SelectedChart)
				MOD(ChartParserModule).ExportChartSet(SelectedChart);

			ImGui::Separator();

			if (ImGui::MenuItem("Set Background", "CTRL+B") && SelectedChart)
			{
				MOD(DialogModule).OpenFileDialog(".png;.jpg", [this](const std::string &InPath)
				{
					MOD(ChartParserModule).SetBackground(SelectedChart, InPath);
					MOD(BackgroundModule).LoadBackground(SelectedChart->BackgroundPath);
				});
			}

			/*if (ImGui::MenuItem("Export", "CTRL+E"))
			{
				
			}*/

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if ( ImGui::MenuItem("Undo", "CTRL+Z") && SelectedChart && SelectedChart->Undo())
				PUSH_NOTIFICATION("Undo");
				
			ImGui::Separator();

			if (ImGui::MenuItem("Copy", "CTRL+C") && SelectedChart)
				MOD(EditModule).OnCopy();

			if (ImGui::MenuItem("Paste", "CTRL+V") && SelectedChart)
				MOD(EditModule).OnPaste();

			if (ImGui::MenuItem("Delete", "DELETE") && SelectedChart)
				MOD(EditModule).OnDelete();

			ImGui::Separator();

			if (ImGui::MenuItem("Mirror", "CTRL+H") && SelectedChart)
				MOD(EditModule).OnMirror();

			if (ImGui::MenuItem("Go To Timepoint", "CTRL+T") && SelectedChart)
				GoToTimePoint();
				
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Options"))
		{
			std::string togglePitch = "Toggle Pitch (";
			togglePitch += MOD(AudioModule).UsePitch ? "Pitched)" : "Stretched)";

			if(ImGui::Button(togglePitch.c_str()))
			{
				MOD(AudioModule).ResetSpeed();
				MOD(AudioModule).UsePitch = !MOD(AudioModule).UsePitch;

				PUSH_NOTIFICATION("Speed Reset");
			}

			if(ImGui::Checkbox("Show Column Lines", &MOD(TimefieldRenderModule).GetSkin().ShowColumnLines))
			{
				if(!SelectedChart)
				{
					MOD(TimefieldRenderModule).GetSkin().ShowColumnLines = false;
					PUSH_NOTIFICATION("Open a Chart First");
				}
			}

			ImGui::Checkbox("Show Waveform", &ShowWaveform);
			ImGui::Checkbox("Use Auto Timing", &EditMode::static_Flags.UseAutoTiming);

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("Shortcuts", "F1"))
				ShowShortCuts();

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

void Program::SetUpMetadata() 
{
	std::string titleName = "New Chart";
	if (!ShouldSetUpNewChart){
		MOD(ChartParserModule).GetChartMetadata(SetUpChartMetadata, SelectedChart);
		titleName = "Edit Metadata";
	}

	MOD(PopupModule).OpenPopup(titleName, [this](bool& OutOpen)
	{
			ImGui::Text("Meta Data");
			ImGui::InputText("Artist", &SetUpChartMetadata.Artist);
			ImGui::InputText("Song Title", &SetUpChartMetadata.SongTitle);
			ImGui::InputText("Charter", &SetUpChartMetadata.Charter);
			ImGui::InputText("Difficulty Name", &SetUpChartMetadata.DifficultyName);

			ImGui::NewLine();

			ImGui::Text("Difficulty");
			if (ShouldSetUpNewChart) ImGui::SliderInt("Key Amount", &SetUpChartMetadata.KeyAmount, 4, 10);
			else {
				std::string key = "Key = ";
				key.append(std::to_string(SelectedChart->KeyAmount));
				ImGui::Text(key.c_str());
			}

			ImGui::DragFloat("OD", &SetUpChartMetadata.OD, 0.1f, 1.0f, 10.0f);
			ImGui::DragFloat("HP", &SetUpChartMetadata.HP, 0.1f, 1.0f, 10.0f);

			ImGui::NewLine();

			std::string audioButtonName =  SetUpChartMetadata.AudioPath == "" ? "Pick an audio file" : SetUpChartMetadata.AudioPath;
			std::string chartButtonName =  SetUpChartMetadata.ChartFolderPath == "" ? "Pick a chart folder path" : SetUpChartMetadata.ChartFolderPath;
			std::string backgroundButtonName =  SetUpChartMetadata.BackgroundPath == "" ? "Pick a background (optional)" : SetUpChartMetadata.BackgroundPath;
			
			ImGui::Text("Relevant Paths");
			
			if(ImGui::Button(audioButtonName.c_str()))
			{
				ShouldSetUpMetadata = OutOpen = false;

				MOD(DialogModule).OpenFileDialog(".mp3;.ogg;.wav", [this](const std::string &InPath) 
				{
					SetUpChartMetadata.AudioPath = InPath;
					ShouldSetUpMetadata = true;
				}, true);
			}

			if(ImGui::Button(chartButtonName.c_str()))
			{
				ShouldSetUpMetadata = OutOpen = false;

				MOD(DialogModule).OpenFolderDialog([this](const std::string &InPath) 
				{
					SetUpChartMetadata.ChartFolderPath = InPath;
					ShouldSetUpMetadata = true;
				}, true);
			}

			if(ImGui::Button(backgroundButtonName.c_str()))
			{
				ShouldSetUpMetadata = OutOpen = false;

				MOD(DialogModule).OpenFileDialog(".png;.jpg", [this](const std::string &InPath) 
				{
					SetUpChartMetadata.BackgroundPath = InPath;
					ShouldSetUpMetadata = true;
				}, true);
			}

			ImGui::NewLine();

			if(ShouldSetUpNewChart)
			{
				if(ImGui::Button("Create"))
				{
					OpenChart(MOD(ChartParserModule).CreateNewChart(SetUpChartMetadata));

					ShouldSetUpMetadata = ShouldSetUpNewChart = OutOpen = false;
					SetUpChartMetadata = ChartMetadata();
				}
			}

			else
			{
				if(ImGui::Button("Save"))
				{
					OpenChart(MOD(ChartParserModule).SetChartMetadata(SelectedChart, SetUpChartMetadata));

					ShouldSetUpMetadata = ShouldSetUpNewChart = OutOpen = false;
					SetUpChartMetadata = ChartMetadata();
				}
			}

			ImGui::SameLine();
			
			if(ImGui::Button("Close"))
			{
				ShouldSetUpMetadata = ShouldSetUpNewChart = OutOpen = false;
				SetUpChartMetadata = ChartMetadata();
			}
	});
}

void Program::ShowShortCuts() 
{
	MOD(PopupModule).OpenPopup("New Chart", [this](bool& OutOpen)
	{
		ImGui::Text("Select Edit Mode"); ImGui::SameLine(196.f); ImGui::Text("NUM 1");
		ImGui::Text("Select Area"); ImGui::SameLine(196.f); ImGui::Text("Left Click+Drag");
		
		ImGui::Spacing();
		
		ImGui::Text("Note Edit Mode"); ImGui::SameLine(196.f); ImGui::Text("NUM 2");
		ImGui::Text("Place Note"); ImGui::SameLine(196.f); ImGui::Text("Left Click");
		ImGui::Text("Remove Note"); ImGui::SameLine(196.f); ImGui::Text("Right Click");
		ImGui::Text("Place Hold"); ImGui::SameLine(196.f); ImGui::Text("SHIFT+Left Click+Drag");
		
		ImGui::Spacing();
		
		ImGui::Text("Timing Edit Mode"); ImGui::SameLine(196.f); ImGui::Text("NUM 3");
		ImGui::Text("Place BPM Node"); ImGui::SameLine(196.f); ImGui::Text("Left Click");
		ImGui::Text("Remove BPM Node"); ImGui::SameLine(196.f); ImGui::Text("Right Click");
		ImGui::Text("Snap BPM Node"); ImGui::SameLine(196.f); ImGui::Text("SHIFT");
		
		ImGui::Spacing();
		
		ImGui::Text("Scroll"); ImGui::SameLine(196.f); ImGui::Text("Mouse Wheel");
		ImGui::Text("Zoom"); ImGui::SameLine(196.f); ImGui::Text("CTRL+Mouse Wheel");
		ImGui::Text("Audio Playback Speed"); ImGui::SameLine(196.f); ImGui::Text("SHIFT+Mouse Wheel");
		ImGui::Text("Snap Division"); ImGui::SameLine(196.f); ImGui::Text("ALT+Mouse Wheel");

		ImGui::Spacing();

		ImGui::Text("Undo"); ImGui::SameLine(196.f); ImGui::Text("CTRL+Z");
		ImGui::Text("Copy"); ImGui::SameLine(196.f); ImGui::Text("CTRL+C");
		ImGui::Text("Paste"); ImGui::SameLine(196.f); ImGui::Text("CTRL+V");
		ImGui::Text("Delete"); ImGui::SameLine(196.f); ImGui::Text("DELETE");
		ImGui::Text("Mirror"); ImGui::SameLine(196.f); ImGui::Text("CTRL+H");
		ImGui::Text("Go To Timepoint"); ImGui::SameLine(196.f); ImGui::Text("CTRL+V");
		
		ImGui::Spacing();

		ImGui::Text("New Chart"); ImGui::SameLine(196.f); ImGui::Text("CTRL+N");
		ImGui::Text("Open"); ImGui::SameLine(196.f); ImGui::Text("CTRL+O");
		ImGui::Text("Save"); ImGui::SameLine(196.f); ImGui::Text("CTRL+S");
		ImGui::Text("Set Background"); ImGui::SameLine(196.f); ImGui::Text("CTRL+B");
		
		ImGui::NewLine();
		
		if(ImGui::Button("close") || MOD(InputModule).WasKeyPressed(sf::Keyboard::Escape))
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

	if(MOD(InputModule).IsDeleting())
		MOD(EditModule).OnDelete();	

	if (MOD(InputModule).IsCtrlKeyDown())
	{
		if(MOD(InputModule).WasKeyPressed(sf::Keyboard::Key::Z))
		{
			if(SelectedChart->Undo())
				PUSH_NOTIFICATION("Undo");
		}

		if (MOD(InputModule).IsScrollingUp())
			return ApplyDeltaToZoom(0.1f);

		if (MOD(InputModule).IsScrollingDown())
			return ApplyDeltaToZoom(-0.1f);

		if (MOD(InputModule).WasKeyPressed(sf::Keyboard::Key::C))
			return void(MOD(EditModule).OnCopy());

		if (MOD(InputModule).WasKeyPressed(sf::Keyboard::Key::V))
			return void(MOD(EditModule).OnPaste());

		if (MOD(InputModule).WasKeyPressed(sf::Keyboard::Key::H))
			return void(MOD(EditModule).OnMirror());

		if (MOD(InputModule).WasKeyPressed(sf::Keyboard::Key::A))
			return void(MOD(EditModule).OnSelectAll());

		if (MOD(InputModule).WasKeyPressed(sf::Keyboard::Key::T))
			GoToTimePoint();

		if (MOD(InputModule).WasKeyPressed(sf::Keyboard::Key::B))
		{
			MOD(DialogModule).OpenFileDialog(".png;.jpg", [this](const std::string &InPath)
			{
				MOD(ChartParserModule).SetBackground(SelectedChart, InPath);
				MOD(BackgroundModule).LoadBackground(SelectedChart->BackgroundPath);
			});
		}
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
		if (MOD(InputModule).IsScrollingUp())
			return MOD(AudioModule).ChangeSpeed(0.05f), PUSH_NOTIFICATION_LIFETIME(0.5f, "Speed %fx", MOD(AudioModule).GetPlaybackSpeed());

		if (MOD(InputModule).IsScrollingDown())
			return MOD(AudioModule).ChangeSpeed(-0.05f), PUSH_NOTIFICATION_LIFETIME(0.5f, "Speed %fx", MOD(AudioModule).GetPlaybackSpeed());
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

void Program::GlobalInputActions() 
{
	if (MOD(InputModule).WasKeyPressed(sf::Keyboard::F1))
		ShowShortCuts();

	if (MOD(InputModule).IsCtrlKeyDown())
	{
		if (SelectedChart && MOD(InputModule).WasKeyPressed(sf::Keyboard::Key::S))
			MOD(ChartParserModule).ExportChartSet(SelectedChart);

		if(MOD(InputModule).WasKeyPressed(sf::Keyboard::Key::O))
		{
			MOD(DialogModule).OpenFileDialog(".osu", [this](const std::string &InPath) 
			{
				OpenChart(InPath);
			});
		}

		if(MOD(InputModule).WasKeyPressed(sf::Keyboard::Key::N)){
			ShouldSetUpNewChart = true;
			ShouldSetUpMetadata = true;
		}
	}
}

void Program::OpenChart(const std::string InPath) 
{
	SelectedChart = MOD(ChartParserModule).ParseAndGenerateChartSet(InPath);

	MOD(BeatModule).AssignNotesToSnapsInChart(SelectedChart);
	MOD(AudioModule).LoadAudio(SelectedChart->AudioPath);
	MOD(EditModule).SetChart(SelectedChart);
	MOD(BackgroundModule).LoadBackground(SelectedChart->BackgroundPath);
	MOD(TimefieldRenderModule).SetKeyAmount(SelectedChart->KeyAmount);
	MOD(MiniMapModule).Generate(SelectedChart, MOD(TimefieldRenderModule).GetSkin(), MOD(AudioModule).GetSongLengthMilliSeconds());
	MOD(WaveFormModule).SetWaveFormData(MOD(AudioModule).GenerateAndGetWaveformData(SelectedChart->AudioPath), MOD(AudioModule).GetSongLengthMilliSeconds());
	MOD(MiniMapModule).Generate(SelectedChart, MOD(TimefieldRenderModule).GetSkin(), MOD(AudioModule).GetSongLengthMilliSeconds());

	SelectedChart->RegisterOnModifiedCallback([this](TimeSlice &InTimeSlice) 
	{
		//TODO: replicate the timeslice method to optimize when "re-generating"
		MOD(BeatModule).AssignNotesToSnapsInTimeSlice(SelectedChart, InTimeSlice);
		MOD(MiniMapModule).GeneratePortion(InTimeSlice, MOD(TimefieldRenderModule).GetSkin());
	});
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