#pragma once

#include "base/module.h"

class InputModule : public Module
{
public:

	virtual bool ProcessEvent(const sf::Event& InEvent) override;

public:

	bool IsScrollingUp();
	bool IsScrollingDown();

	bool IsTogglingPause();

	bool IsDeleting();

	bool WasKeyPressed(const sf::Keyboard::Key InKey);
	
	bool WasMouseButtonPressed(const sf::Mouse::Button InButton);
	bool WasMouseButtonReleased(const sf::Mouse::Button InButton);

	bool IsCtrlKeyDown();
	bool IsShiftKeyDown();
	bool IsAltKeyDown();
	bool IsUpKeyPressed();
	bool IsDownKeyPressed();

private:

	bool _KeyboardPressedStates[sf::Keyboard::KeyCount];

	bool _MousePressedStates[sf::Mouse::ButtonCount];
	bool _MouseReleasedStates[sf::Mouse::ButtonCount];

	bool _CtrlKey = false;
	bool _ShiftKey = false;
	bool _AltKey = false;
	
	int _MouseWheelDelta = 0;

	bool _GainedFocusLastFrame = false;
};