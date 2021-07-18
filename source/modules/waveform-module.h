#pragma once

#include "base/module.h"

#include <SFML/Graphics.hpp>
#include <map>

class WaveFormModule : public Module
{
public:

    virtual bool StartUp() override;

public:

    void SetWaveFormData(WaveFormData* const InWaveFormData, const Time InSongLengthMilliSeconds);
    void RenderWaveForm(TimefieldRenderGraph& InOutRenderGraph, const Time InTimeBegin, const Time InTimeEnd, const int InScreenX, const float InZoomLevel, const float InWindowHeight);

private:

    const int _WaveFormWidth = 256;
    sf::VertexArray _WaveFormLines;

    sf::RenderTexture _ScalableWaveFormTexture;
    sf::Sprite _ScalableWaveFormSprite;

    WaveFormData* _WaveFormData = nullptr;

    Time _SongLengthMilliSeconds = 0;
};