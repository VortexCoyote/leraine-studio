#pragma once

#include "base/module.h"

class NotificationModule : public Module
{
public:
	
    bool Tick(const float& InDeltaTime) override;
    virtual bool RenderBack(sf::RenderTarget* const InOutRenderTarget);

public:

    void SetStartY(const int InY);

private:

    int _StartY = 0;
	sf::RenderWindow* _RenderWindow = nullptr;
};