#include "shortcut-menu-module.h"

#include "imgui.h"

void ShortcutMenuModule::ShowCheatSheet() 
{
	//TODO: auto-generate this
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
}

bool ShortcutMenuModule::MenuItem(const std::string InLabel, sf::Keyboard::Key InModifierKey, sf::Keyboard::Key InActionKey) 
{
	if(_ImGuiMenuBegin)
	{
        std::string actionKey;

		if(InActionKey == sf::Keyboard::Key::Delete)
			actionKey = "DELETE";
		else if(InActionKey == sf::Keyboard::Key::F1)
			actionKey = "F1";
		else
			actionKey = std::string(1, char(InActionKey - sf::Keyboard::A + 'A'));

		std::string modifierKey = "";

		if(InModifierKey == sf::Keyboard::Key::LControl)
			modifierKey = "CTRL+";

		if(InModifierKey == sf::Keyboard::Key::LShift)
			modifierKey = "SHIFT+";

		if(InModifierKey == sf::Keyboard::Key::LAlt)
			modifierKey = "ALT+";

		if(ImGui::MenuItem(InLabel.c_str(), (modifierKey + actionKey).c_str()))
			return true;
	}

	if(_ShouldCaptureShortcut && InActionKey != sf::Keyboard::Key::Unknown)
	{	
		bool actionKeyDown = _KeyboardPressedStates[InActionKey];

		if(actionKeyDown)
			_KeyboardPressedStates[InActionKey] = false;

		if(_AltKey && InModifierKey == sf::Keyboard::Key::LAlt)
			return actionKeyDown;

		else if(_CtrlKey && InModifierKey == sf::Keyboard::Key::LControl)
			return actionKeyDown;

		else if(_ShiftKey && InModifierKey == sf::Keyboard::Key::LShift)
			return actionKeyDown;

		else 
			return actionKeyDown;
	}

	return false;
}

void ShortcutMenuModule::EndMenu() 
{
	if(_ImGuiMenuBegin)
	{
		_ImGuiMenuBegin = false;
		ImGui::EndMenu();
	}
}

void ShortcutMenuModule::Separator() 
{
	if(_ImGuiMenuBegin)
		ImGui::Separator();
}

bool ShortcutMenuModule::ProcessEvent(const sf::Event& InEvent) 
{
	if (InEvent.type == sf::Event::KeyPressed)
	{
		_ShouldCaptureShortcut = true;

		_KeyboardPressedStates[InEvent.key.code] = true;

		if(InEvent.key.code == sf::Keyboard::LAlt)
			_AltKey = true;

		if(InEvent.key.code == sf::Keyboard::LControl)
			_CtrlKey = true;

		if(InEvent.key.code == sf::Keyboard::LShift)
			_ShiftKey = true;
	}
	else if (InEvent.type == sf::Event::KeyReleased)
	{
		_KeyboardPressedStates[InEvent.key.code] = false;

		if(InEvent.key.code == sf::Keyboard::LAlt)
			_AltKey = false;

		if(InEvent.key.code == sf::Keyboard::LControl)
			_CtrlKey = false;

		if(InEvent.key.code == sf::Keyboard::LShift)
			_ShiftKey = false;
	}

	return true;
}

bool ShortcutMenuModule::BeginMenu(const std::string InLabel) 
{
	if(ImGui::BeginMenu(InLabel.c_str()))
		_ImGuiMenuBegin = true;
	else
		_ImGuiMenuBegin = false;

	return _ImGuiMenuBegin || _ShouldCaptureShortcut;
}
