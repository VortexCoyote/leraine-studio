#pragma once

#include "base/module.h"

class ShortcutMenuModule : public Module
{
public:

    virtual bool ProcessEvent(const sf::Event& InEvent) override;

public:

    bool BeginMenu(const std::string InLabel);
    bool MenuItem(const std::string InLabel, sf::Keyboard::Key InModifierKey, sf::Keyboard::Key InActionKey);
    void EndMenu();
    void Separator();
    
    void ShowCheatSheet();

private:

    bool _ImGuiMenuBegin = false;
    bool _ShouldCaptureShortcut = false;

    bool _CtrlKey = false;
	bool _ShiftKey = false;
	bool _AltKey = false;

    bool _KeyboardPressedStates[sf::Keyboard::KeyCount];
};