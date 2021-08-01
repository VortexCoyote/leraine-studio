#pragma once

#include "base/module.h"

class DebugModule : public Module
{
public:

    void RenderTimeSliceBoundaries(TimefieldRenderGraph& OutRenderGraph, Chart* const InSelectedChart, Time InTimeBegin, Time InTimeEnd);

public:

    bool ShowTimeSliceBoundaries = false;
};