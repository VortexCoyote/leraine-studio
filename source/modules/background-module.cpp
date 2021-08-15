#include "background-module.h"

bool BackgroundModule::RenderBack(sf::RenderTarget* const InOutRenderTarget) 
{
	float procentualChange = float(InOutRenderTarget->getView().getSize().y) / float(_BackgroundTexture.getSize().y);
	
	_BackgroundSprite.setScale(procentualChange, procentualChange);

	const auto x = (float(InOutRenderTarget->getView().getSize().x) - float(_BackgroundTexture.getSize().x) * procentualChange) / 2.f;
	_BackgroundSprite.setPosition(x, 0);
	
	InOutRenderTarget->draw(_BackgroundSprite);

    return true;
}

void BackgroundModule::LoadBackground(const std::filesystem::path& InPath) 
{
	_BackgroundTexture = sf::Texture();
	_BackgroundSprite = sf::Sprite();

    _BackgroundTexture.loadFromFile(InPath.string());
	_BackgroundSprite.setTexture(_BackgroundTexture);
}