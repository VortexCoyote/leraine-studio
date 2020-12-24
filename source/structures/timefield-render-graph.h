#pragma once

#include <SFML/Graphics.hpp>

#include "chart.h"
#include "timefield-metrics.h"

struct NoteRenderCommand
{
    NoteRenderCommand(const Note& InNote, const Column InColumn, const sf::Int8 InAlpha);

    const Column NoteColumn;
    const Note RenderNote;

    const sf::Int8 Alpha;
};

struct TimefieldRenderCommand
{
    TimefieldRenderCommand(const Column InColumn, const Time InTime, const std::function<void(sf::RenderTarget* const, const TimefieldMetrics&, int, int)>& InRenderWork);

    const Column ColumnPoint;
    const Time TimePoint;

    const std::function<void(sf::RenderTarget* const, const TimefieldMetrics&, int, int)> RenderWork;
};

class TimefieldRenderGraph
{
public:

    TimefieldRenderGraph();

    void Render(std::function<void(const NoteRenderCommand&)> InWork);
    void Render(std::function<void(const TimefieldRenderCommand&)> InWork);

public:

    void SubmitCommonNoteRenderCommand(const Column InColumn, const Time InTime, const int InBeatSnap = -1, const sf::Int8 InAlpha = 255);
    void SubmitHoldNoteRenderCommand(const Column InColumn, const Time InTimeBegin, const Time InTimeEnd, const int InBeatSnapBegin = -1, const int InBeatSnapEnd = -1, const sf::Int8 InAlpha = 255);
    void SubmitNoteRenderCommand(const Note& InNote, const Column InColumn, const sf::Int8 InAlpha = 255);

    void SubmitTimefieldRenderCommand(const Column InColumn, const Time InTime, const std::function<void(sf::RenderTarget* const, const TimefieldMetrics&, int, int)>& InRenderWork);

private:

    std::vector<NoteRenderCommand> _NoteRenderCommands;
    std::vector<TimefieldRenderCommand> _TimefieldRenderCommands;
};