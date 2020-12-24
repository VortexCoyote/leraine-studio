#pragma once

#include <SFML/Graphics.hpp>

#include "base/module.h"

class BackgroundModule : public Module
{
public:

    virtual bool RenderBack(sf::RenderTarget* const InOutRenderTarget) override;

public:

    void LoadBackground(const std::string& InPath);

private:
    
    sf::Texture _BackgroundTexture;
	sf::Sprite _BackgroundSprite;
};