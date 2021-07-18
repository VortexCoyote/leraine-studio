#pragma once

#include "base/module.h"

#include <SFML/Graphics.hpp>

class MiniMapModule : public Module
{
public:

    bool Tick(const float& InDeltaTime) override;

public:

    void Generate(Chart* const InChart, Skin& InSkin, const Time InSongLength);
    TimefieldRenderGraph& GetPreviewRenderGraph(Chart* const InChart);

    bool IsHoveringTimeline(const int InScreenX, const int InScreenY, const int InHeight, const int InDistanceFromBorders, const Time InTime,  const Time InTimeScreenBegin, const Time InTimeScreenEnd, const Cursor& InCursor);
    bool IsPossibleToDrag();
    bool IsDragging();
    bool ShouldPreview();

    void StartDragging();
    void EndDragging();

    Time GetHoveredTime();

    void Render(sf::RenderTarget* InOutRenderTarget);
    void RenderPreview(sf::RenderTarget* InOutRenderTarget, sf::RenderTexture* InPreviewRenderTexture);

private:

    int _Width = 0;
    int _ScaledSongLength = 0;

    int _MiniScreenBottomPosition = 0;
    int _MiniScreenHeight = 1;

    int _SongPositionOnMiniMap = 0;
    Time _SongLength = 0;
    Time _HoveredTime = 0;
    Time _CurrentTime = 0;

    int _CursorScreenY = 0;
    int _TimelinePositionTop = 0;
    int _TimelinePositionBottom = 0;
    bool _ShouldPreview = false;

    bool _IsPossibleToDrag = false;
    bool _IsDragging = false;

    const int _HeightScale = 150;
    const int _NoteWidth = 2;
    const int _NoteHeight = 1;
    const int _BorderPadding = 4;
    const int _DragButtonBounds = 20;
    const Time _PreviewTimeLength = 1500;

    sf::RectangleShape _MiniMapRectangle;

    sf::RenderTexture _MiniMapRenderTexture;
    sf::Sprite _MiniMapSprite;

    TimefieldRenderGraph _PreviewRenderGraph;

    sf::RenderTexture _PreviewRenderTexture;
    sf::Sprite _PreviewSprite;
};