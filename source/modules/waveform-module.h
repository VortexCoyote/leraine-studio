#pragma once

#include "base/module.h"

#include <SFML/Graphics.hpp>
#include <map>

class WaveFormModule : public Module
{
public:

    virtual bool StartUp() override;

public:

    void GenerateWaveForm(WaveFormData* const InWaveFormData, const Time InSongLengthMilliSeconds);
    void RenderWaveForm(sf::RenderTarget* const InOutRenderTarget, const Time InTimeBegin, const Time InTimeEnd, const int InScreenX, const float InZoomLevel);

private:

    const int _SliceLength = 1024;
    const int _WaveFormWidth = 316;

    sf::Sprite _WaveFormSliceSprite;
    std::map<Time, sf::RenderTexture> _WaveFormSlices;
};