#pragma once

#include "base/module.h"

class ImGuiModule : public Module
{
public:

	bool Tick(const float& InDeltaTime) override;
	bool RenderFront(sf::RenderTarget* const InOutRenderTarget) override;
	bool ProcessEvent(const sf::Event & InEvent) override;

	bool ShutDown() override;

public:

	void Init(sf::RenderWindow* const InOutRenderWindow);
	void ApplyTheme();

private:

	sf::RenderWindow* _RenderWindow = nullptr;
};