#include "minimap-module.h"


void MiniMapModule::Generate(Chart* const InChart, Skin& InSkin, const Time InSongLength)
{
    _Width = _BorderPadding * 2 + _NoteWidth + (InChart->KeyAmount * _NoteWidth + (InChart->KeyAmount - 1)) + _NoteWidth;
    _ScaledSongLength = InSongLength / _HeightScale;
    _SongLength = InSongLength;

    _MiniMapRenderTexture.create(_Width, _ScaledSongLength);
    _MiniMapRenderTexture.clear({0, 0, 0, 196});

    InChart->IterateNotesInTimeRange(0, InSongLength, [this, &InSkin, &InSongLength](Note& InOutNote, const Column InColumn)
    {
        if(InOutNote.Type == Note::EType::HoldEnd || InOutNote.Type == Note::EType::HoldIntermediate)
            return;

        if(InOutNote.Type == Note::EType::HoldBegin)
        {
            sf::RectangleShape rectangleHold;

            rectangleHold.setSize(sf::Vector2f(_NoteWidth, (InOutNote.TimePointEnd - InOutNote.TimePointBegin + _NoteHeight) / _HeightScale));
            rectangleHold.setFillColor({32, 255, 32, 255});

            rectangleHold.setPosition(sf::Vector2f(_BorderPadding + _NoteWidth + (InColumn * _NoteWidth + InColumn), (InOutNote.TimePoint - _NoteHeight) / _HeightScale));

            _MiniMapRenderTexture.draw(rectangleHold);
        }

        sf::RectangleShape rectangle;

        rectangle.setSize(sf::Vector2f(_NoteWidth, _NoteHeight));
        rectangle.setFillColor(InSkin.SnapColorTable[InOutNote.BeatSnap]);

        rectangle.setPosition(sf::Vector2f(_BorderPadding + _NoteWidth + (InColumn * _NoteWidth + InColumn), (InOutNote.TimePoint - _NoteHeight) / _HeightScale));

        _MiniMapRenderTexture.draw(rectangle);
    });

    _MiniMapSprite = sf::Sprite();

    _MiniMapSprite.setTexture(_MiniMapRenderTexture.getTexture());
}

bool MiniMapModule::IsHoveringTimeline(const int InScreenX, const int InScreenY, const int InHeight, const int InDistanceFromBorders, const Time InTime, const Time InTimeScreenBegin, const Time InTimeScreenEnd, const Cursor& InCursor) 
{
    int resultedHeight = InHeight - InDistanceFromBorders  *2;

    _MiniMapRectangle.setPosition(InScreenX, InScreenY + InDistanceFromBorders);
    _MiniMapRectangle.setSize(sf::Vector2f((float)_MiniMapRenderTexture.getTexture().getSize().x, (float)resultedHeight));

    float percentualDeltaMiniMap = float(resultedHeight) / float(_ScaledSongLength); 
    _SongPositionOnMiniMap = resultedHeight - (InTime / _HeightScale) * percentualDeltaMiniMap;

    _MiniScreenBottomPosition = resultedHeight - (InTimeScreenBegin / _HeightScale) * percentualDeltaMiniMap;
    _MiniScreenHeight = (float(InTimeScreenBegin) / float(_HeightScale)) - (float(InTimeScreenEnd) / float(_HeightScale));
    

    sf::IntRect subRectangle;

    subRectangle.height = resultedHeight;
    subRectangle.width = _MiniMapRenderTexture.getTexture().getSize().x;

    subRectangle.left = 0;

    float timeOffset = (InTime / _HeightScale) - (InTime / _HeightScale) * percentualDeltaMiniMap;
    subRectangle.top = (_MiniMapRenderTexture.getTexture().getSize().y - subRectangle.height) - timeOffset;

    _MiniMapSprite.setTextureRect(subRectangle);


    if(InCursor.X >= InScreenX && InCursor.X <= InScreenX + _MiniMapRectangle.getSize().x 
    && InCursor.Y >= InScreenY && InCursor.Y <= InScreenY + resultedHeight)
    {
        int relativeMiniMapTime = InCursor.Y - InDistanceFromBorders * 2;
		_HoveredTime = float(resultedHeight - relativeMiniMapTime) / resultedHeight * float(_SongLength);

        return true;
    }

    return false;
}

Time MiniMapModule::GetHoveredTime() 
{
    return _HoveredTime;
}

void MiniMapModule::Render(sf::RenderTarget* InRenderTarget) 
{
    _MiniMapRectangle.setFillColor({0, 0, 0, 0});
    _MiniMapRectangle.setOutlineColor({255, 255, 255, 255});
    _MiniMapRectangle.setOutlineThickness(1.0f);

    _MiniMapSprite.setPosition(_MiniMapRectangle.getPosition().x, _MiniMapRectangle.getPosition().y);

    sf::RectangleShape screenView;

    screenView.setPosition(_MiniMapRectangle.getPosition().x, _MiniMapRectangle.getPosition().y + _MiniScreenBottomPosition);
    screenView.setSize(sf::Vector2f(_MiniMapRectangle.getSize().x, _MiniScreenHeight));
    screenView.setFillColor({255, 255, 255, 64});
    screenView.setOutlineColor({255, 255, 255, 255});
    screenView.setOutlineThickness(1.0f);

    InRenderTarget->draw(_MiniMapSprite);
    InRenderTarget->draw(_MiniMapRectangle);
    InRenderTarget->draw(screenView);
}