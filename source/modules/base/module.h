#pragma once

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "../../global/global-types.h"
#include "../../global/global-functions.h"

class Module
{
public:

	Module();
	~Module();

	virtual bool StartUp();

	virtual bool Tick(const float& InDeltaTime);
	virtual bool RenderBack(sf::RenderTarget* const InOutRenderTarget);
	virtual bool RenderFront(sf::RenderTarget* const InOutRenderTarget);
	virtual bool ProcessEvent(const sf::Event& InEvent);

	virtual bool ShutDown();
};