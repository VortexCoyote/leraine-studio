#include "skin.h"

#include <algorithm>
#include <functional>
#include <fstream>

#include <SFML/Graphics.hpp>

void Skin::LoadResources(const int InKeyAmount, const std::filesystem::path& InSkinFolderPath)
{
	ResetTexturesAndSprites();

	//TODO: serialize this
	SnapColorTable[1] = sf::Color(255, 0, 0, 255);
	SnapColorTable[2] = sf::Color(0, 0, 255, 255);
	SnapColorTable[3] = sf::Color(138, 43, 226, 255);
	SnapColorTable[4] = sf::Color(255, 255, 0, 255);
	SnapColorTable[5] = sf::Color(75, 75, 75, 255);
	SnapColorTable[6] = sf::Color(255, 105, 180, 255);
	SnapColorTable[8] = sf::Color(255, 128, 0, 255);
	SnapColorTable[12] = sf::Color(0, 255, 255, 255);
	SnapColorTable[16] = sf::Color(0, 128, 0, 255);
	SnapColorTable[24] = sf::Color(75, 75, 75, 255);
	SnapColorTable[48] = sf::Color(75, 75, 75, 255);

	SnapColorTable[-1] = sf::Color(75, 75, 75, 255);

	std::filesystem::path path = InSkinFolderPath;
	path += "/" + std::to_string(InKeyAmount) + "k/";
	path.make_preferred();

	std::filesystem::path defaultHoldBodyPath = path;
	defaultHoldBodyPath += "holdbody.png";
	defaultHoldBodyPath.make_preferred();

	std::filesystem::path defaultHoldBodyCapPath = path;
	defaultHoldBodyCapPath += "holdcap.png";
	defaultHoldBodyCapPath.make_preferred();

	for (int key = 0; key < InKeyAmount; key++)
	{
		std::filesystem::path subPath = path;
		subPath /= "column_" + std::to_string(key + 1) + ".png";

		NoteTextures[key].loadFromFile(subPath.make_preferred().string());

		subPath = path;
		subPath /= "column_" + std::to_string(key + 1) + "_overlay.png";

		if (_HasOverlay = !!std::ifstream(subPath))
			NoteOverlayTextures[key].loadFromFile(subPath.make_preferred().string());
	
		subPath = path;
		subPath /= "column_" + std::to_string(key + 1) + "_holdbody.png";

		if (!HoldBodyTextures[key].loadFromFile(subPath.make_preferred().string()))
			HoldBodyTextures[key].loadFromFile(defaultHoldBodyPath.string());

		subPath = path;
		subPath /= "column_" + std::to_string(key + 1) + "_holdcap.png";

		if (!HoldBodyCapTextures[key].loadFromFile(subPath.make_preferred().string()))
			HoldBodyCapTextures[key].loadFromFile(defaultHoldBodyCapPath.string());
	}

	// HitlineTexture.loadFromFile(hitlinePath.string());
	// HitlineSprite.setTexture(HitlineTexture);
}

void Skin::RenderNote(const int InColumn, const int InPositionY, sf::RenderTarget* InOutRenderTarget, const int InBeatSnap, const sf::Int8 InAlpha)
{
	sf::Color snapColor = SnapColorTable[InBeatSnap];
	snapColor.a = InAlpha;

	NoteSprite.setColor(snapColor);

	NoteSprite.setTexture(NoteTextures[InColumn]);
	NoteSprite.setScale((float)_TimefieldMetrics.ColumnSize / (float)NoteTextures[InColumn].getSize().x,
					    (float)_TimefieldMetrics.ColumnSize / (float)NoteTextures[InColumn].getSize().y);

	NoteSprite.setPosition(_TimefieldMetrics.FirstColumnPosition + InColumn * _TimefieldMetrics.ColumnSize, InPositionY - _TimefieldMetrics.ColumnSize);
	InOutRenderTarget->draw(NoteSprite);

	if (!_HasOverlay)
		return;

	NoteSprite.setColor(sf::Color(255, 255, 255, InAlpha));

	NoteSprite.setTexture(NoteOverlayTextures[InColumn]);
	NoteSprite.setScale((float)_TimefieldMetrics.ColumnSize / (float)NoteTextures[InColumn].getSize().x,
		(float)_TimefieldMetrics.ColumnSize / (float)NoteTextures[InColumn].getSize().y);

	NoteSprite.setPosition(_TimefieldMetrics.FirstColumnPosition + InColumn * _TimefieldMetrics.ColumnSize, InPositionY - _TimefieldMetrics.ColumnSize);
	InOutRenderTarget->draw(NoteSprite);
}

void Skin::RenderHoldBody(const int InColumn, const int InPositionY, const int InHeight, sf::RenderTarget* InOutRenderTarget, const sf::Int8 InAlpha)
{
	sf::Color color = {255, 255, 255, 255};
	color.a = InAlpha;

	HoldBodySprite.setColor(color);

	HoldBodySprite.setTexture(HoldBodyTextures[InColumn]);
	HoldBodySprite.setScale((float)_TimefieldMetrics.ColumnSize / (float)HoldBodyTextures[InColumn].getSize().x,
							(float)(std::max(0, InHeight - int(_TimefieldMetrics.ColumnSize / 2))) / (float)HoldBodyTextures[InColumn].getSize().y);
	
	HoldBodySprite.setPosition(_TimefieldMetrics.FirstColumnPosition + InColumn * _TimefieldMetrics.ColumnSize, InPositionY);
	InOutRenderTarget->draw(HoldBodySprite);
}

void Skin::RenderHoldCap(const int InColumn, const int InPositionY, sf::RenderTarget* InOutRenderTarget, const sf::Int8 InAlpha)
{	
	sf::Color color = {255, 255, 255, 255};
	color.a = InAlpha;
	
	HoldCapSprite.setColor(color);

	HoldCapSprite.setTexture(HoldBodyCapTextures[InColumn]);
	HoldCapSprite.setScale((float)_TimefieldMetrics.ColumnSize / (float)HoldBodyCapTextures[InColumn].getSize().x,
						   (float)_TimefieldMetrics.ColumnSize / (float)HoldBodyCapTextures[InColumn].getSize().y);
	
	HoldCapSprite.setPosition(_TimefieldMetrics.FirstColumnPosition + InColumn * _TimefieldMetrics.ColumnSize, InPositionY - _TimefieldMetrics.ColumnSize);
	InOutRenderTarget->draw(HoldCapSprite);
}

// unused
// void Skin::RenderHitline(sf::RenderTarget* InOutRenderTarget)
// {
// 	HitlineSprite.setScale((float)_TimefieldMetrics.NoteFieldWidth / (float)HitlineTexture.getSize().x, 1.0);

// 	HitlineSprite.setPosition(_TimefieldMetrics.FirstColumnPosition, _TimefieldMetrics.HitLinePosition);
// 	InOutRenderTarget->draw(HitlineSprite);
// }

void Skin::RenderReceptors(sf::RenderTarget* InRenderTarget, const int InBeatSnap) 
{
	for(int column = 0; column <= _TimefieldMetrics.KeyAmount; ++column)
		RenderNote(column, _TimefieldMetrics.HitLinePosition +_TimefieldMetrics.ColumnSize / 2, InRenderTarget, InBeatSnap, 96);
}

void Skin::RenderTimeFieldBackground(sf::RenderTarget* InOutRenderTarget)
{
	sf::RectangleShape rectangle;
	
	rectangle.setPosition(_TimefieldMetrics.LeftSidePosition, 0);
	rectangle.setSize({ float(_TimefieldMetrics.FieldWidth), float(InOutRenderTarget->getView().getSize().y) });
	rectangle.setFillColor(sf::Color(0, 0, 0, 255));

	InOutRenderTarget->draw(rectangle);

	if(ShowColumnLines)
	{
		for(int i = 0; i <= _TimefieldMetrics.KeyAmount; ++i)
		{
			sf::RectangleShape line;

			line.setPosition(_TimefieldMetrics.FirstColumnPosition + i * _TimefieldMetrics.ColumnSize, 0);
			line.setSize({ 1.0f, float(InOutRenderTarget->getView().getSize().y) });
			line.setFillColor(sf::Color(255, 255, 255, 64));

			InOutRenderTarget->draw(line);
		}
	}
}

void Skin::UpdateTimefieldMetrics(const TimefieldMetrics& InTimefieldMetrics) 
{
	_TimefieldMetrics = InTimefieldMetrics;
}

void Skin::ResetTexturesAndSprites()
{
	//memory managements being lifetime bound actually sucks
	for (size_t i = 0; i < 16; i++)
	{
		NoteTextures[i] = sf::Texture();
		NoteOverlayTextures[i] = sf::Texture();
		HoldBodyTextures[i] = sf::Texture();
		HoldBodyCapTextures[i] = sf::Texture();
	}
	
	SelectTexture = sf::Texture();
	// HitlineTexture = sf::Texture();

	NoteSprite = sf::Sprite();
	HoldBodySprite = sf::Sprite();
	HoldCapSprite = sf::Sprite();

	// HitlineSprite = sf::Sprite();
}