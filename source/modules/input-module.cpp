#include "input-module.h"

bool InputModule::ProcessEvent(const sf::Event& InEvent)
{
	if (_GainedFocusLastFrame)
	{
		_GainedFocusLastFrame = false;
		return false;
	}

	if (InEvent.type == sf::Event::MouseWheelScrolled)
	{
		_MouseWheelDelta = InEvent.mouseWheelScroll.delta;
	}

	else if (InEvent.type == sf::Event::MouseButtonPressed)
	{
		_MousePressedStates[InEvent.mouseButton.button] = true;
		_MouseReleasedStates[InEvent.mouseButton.button] = false;
	}

	else if (InEvent.type == sf::Event::MouseButtonReleased)
	{
		_MousePressedStates[InEvent.mouseButton.button] = false;
		_MouseReleasedStates[InEvent.mouseButton.button] = true;
	}

	else if (InEvent.type == sf::Event::KeyPressed)
	{
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

	else if (InEvent.type == sf::Event::GainedFocus)
		_GainedFocusLastFrame = true;

	return true;
}

bool InputModule::IsScrollingUp()
{
	return _MouseWheelDelta > 0 ? !(_MouseWheelDelta = 0) : false;
}

bool InputModule::IsScrollingDown()
{
	return _MouseWheelDelta < 0 ? !(_MouseWheelDelta = 0) : false;
}

bool InputModule::IsTogglingPause()
{
	return WasKeyPressed(sf::Keyboard::Space);
}

bool InputModule::IsDeleting() 
{
	return WasKeyPressed(sf::Keyboard::Delete);
}

bool InputModule::WasKeyPressed(const sf::Keyboard::Key InKey)
{
	bool pressed = _KeyboardPressedStates[InKey];
	_KeyboardPressedStates[InKey] = false;
	
	return pressed;
}

bool InputModule::WasMouseButtonPressed(const sf::Mouse::Button InButton)
{
	bool pressed = _MousePressedStates[InButton];
	_MousePressedStates[InButton] = false;

	return pressed;
}

bool InputModule::WasMouseButtonReleased(const sf::Mouse::Button InButton)
{
	bool released = _MouseReleasedStates[InButton];
	_MouseReleasedStates[InButton] = false;

	return released;
}

bool InputModule::IsCtrlKeyDown()
{
	return _CtrlKey;
}

bool InputModule::IsShiftKeyDown()
{
	return _ShiftKey;
}

bool InputModule::IsAltKeyDown()
{
	return _AltKey;
}
