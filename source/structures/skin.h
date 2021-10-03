#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <filesystem>
#include <map>

#include "timefield-metrics.h"

struct Skin
{
	void LoadResources(const int InKeyAmount, const std::filesystem::path& InSkinFolderPath);

	void RenderNote(const int InColumn, const int InPositionY, sf::RenderTarget* InRenderTarget, const int InBeatSnap = -1, const sf::Int8 InAlpha = 255);
	void RenderHoldBody(const int InColumn, const int InPositionY, const int InHeight, sf::RenderTarget* InRenderTarget, const sf::Int8 InAlpha = 255);
	void RenderHoldCap(const int InColumn, const int InPositionY, sf::RenderTarget* InRenderTarget, const sf::Int8 InAlpha = 255);

	// void RenderHitline(sf::RenderTarget* InRenderTarget);
	void RenderReceptors(sf::RenderTarget* InRenderTarget, const int InBeatSnap = -1);
	void RenderTimeFieldBackground(sf::RenderTarget* InRenderTarget);

	sf::Texture NoteTextures[16];
	sf::Texture NoteOverlayTextures[16];

	sf::Texture HoldBodyTextures[16];
	sf::Texture HoldBodyCapTextures[16];

	sf::Texture SelectTexture;
	// sf::Texture HitlineTexture;

	sf::Sprite NoteSprite;
	sf::Sprite HoldBodySprite;
	sf::Sprite HoldCapSprite;

	// sf::Sprite HitlineSprite;

	std::map<int, sf::Color> SnapColorTable;

	bool ShowColumnLines = false;

public: //meta methods

	void UpdateTimefieldMetrics(const TimefieldMetrics& InTimefieldMetrics);

private:

	void ResetTexturesAndSprites();

	bool _HasOverlay;

	TimefieldMetrics _TimefieldMetrics;
};