#pragma once

#include "base/module.h"

#include <SFML/Graphics.hpp>

class MiniMapModule : public Module
{
public:

    void Generate(Chart* const InChart, Skin& InSkin, const Time InSongLength);

    bool IsHoveringTimeline(const int InScreenX, const int InScreenY, const int InHeight, const int InDistanceFromBorders, const Time InTime,  const Time InTimeScreenBegin, const Time InTimeScreenEnd, const Cursor& InCursor);
    Time GetHoveredTime();

    void Render(sf::RenderTarget* InRenderTarget);
    

private:

    int _Width = 0;
    int _ScaledSongLength = 0;

    int _MiniScreenBottomPosition = 0;
    int _MiniScreenHeight = 1;

    int _SongPositionOnMiniMap = 0;
    Time _SongLength = 0;

    Time _HoveredTime = 0;

    const int _HeightScale = 150;
    const int _NoteWidth = 2;
    const int _NoteHeight = 1;
    const int _BorderPadding = 4;

    sf::RectangleShape _MiniMapRectangle;

    sf::RenderTexture _MiniMapRenderTexture;
    sf::Sprite _MiniMapSprite;
};