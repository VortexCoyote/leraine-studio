#pragma once

#include "base/module.h"

#include <SFML/Graphics.hpp>


/*
* NOT USED ATM
*/

class WindowModule : public Module
{
public:

    virtual bool StartUp() override;
    virtual bool ProcessEvent(const sf::Event& InEvent) override;
	virtual bool Tick(const float& InDeltaTime) override;

public:

    void Init(sf::RenderWindow* const InOutRenderWindow, bool* OutShouldClose, std::function<void()> InWindowWork);

private:

    sf::RenderWindow* _RenderWindow = nullptr;
    bool _MovingWindow = false;

    int _CurrentWindowWidth = 1024;
    int _CurrentWindowHeight = 768;

    int _PreviousWindowWidth = 0;
    int _PreviousWindowHeight = 0;

    std::function<void()> _WindowWork;
    bool* _ShouldExitProgram;
};