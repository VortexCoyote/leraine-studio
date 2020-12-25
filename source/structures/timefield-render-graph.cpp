#include "timefield-render-graph.h"

void TimefieldRenderGraph::Render(std::function<void(const NoteRenderCommand&)> InWork) 
{
    for(auto& renderCommand : _NoteRenderCommands)
    {
        InWork(renderCommand);
    }
}

void TimefieldRenderGraph::Render(std::function<void(const TimefieldRenderCommand&)> InWork) 
{
    for(auto& timefieldCommand : _TimefieldRenderCommands)
    {
        InWork(timefieldCommand);
    }
}

void TimefieldRenderGraph::ClearRenderCommands() 
{
    _TimefieldRenderCommands.clear();
    _NoteRenderCommands.clear();    
}

void TimefieldRenderGraph::SubmitCommonNoteRenderCommand(const Column InColumn, const Time InTime, const int InBeatSnap, const sf::Int8 InAlpha) 
{
    Note note;

    note.Type = Note::EType::Common;
    note.TimePoint = InTime;
    note.BeatSnap = InBeatSnap;

    SubmitNoteRenderCommand(note, InColumn, InAlpha);
}

void TimefieldRenderGraph::SubmitHoldNoteRenderCommand(const Column InColumn, const Time InTimeBegin, const Time InTimeEnd, const int InBeatSnapBegin, const int InBeatSnapEnd, const sf::Int8 InAlpha)
{
    Note holdBegin;

    holdBegin.Type = Note::EType::HoldBegin;
    holdBegin.TimePoint = InTimeBegin;
    holdBegin.TimePointBegin = InTimeBegin;
    holdBegin.TimePointEnd = InTimeEnd;
    holdBegin.BeatSnap = InBeatSnapBegin;

    SubmitNoteRenderCommand(holdBegin, InColumn, InAlpha);

    Note holdEnd;

    holdEnd.Type = Note::EType::HoldEnd;
    holdEnd.TimePoint = InTimeEnd;
    holdEnd.TimePointBegin = InTimeBegin;
    holdEnd.TimePointEnd = InTimeEnd;
    holdEnd.BeatSnap = InBeatSnapEnd;

    SubmitNoteRenderCommand(holdEnd, InColumn, InAlpha);
}

void TimefieldRenderGraph::SubmitNoteRenderCommand(const Note& InNote, const Column InColumn, const sf::Int8 InAlpha) 
{
    _NoteRenderCommands.emplace_back(InNote, InColumn, InAlpha);
}

void TimefieldRenderGraph::SubmitTimefieldRenderCommand(const Column InColumn, const Time InTime, const std::function<void(sf::RenderTarget* const, const TimefieldMetrics&, int, int)>& InRenderWork) 
{
    _TimefieldRenderCommands.emplace_back(InColumn, InTime, InRenderWork);
}

TimefieldRenderGraph::TimefieldRenderGraph() 
{
    _NoteRenderCommands.reserve(10000);
    _TimefieldRenderCommands.reserve(10000);
}

NoteRenderCommand::NoteRenderCommand(const Note& InNote, const Column InColumn, const sf::Int8 InAlpha) 
    : RenderNote(InNote), NoteColumn(InColumn), Alpha(InAlpha)
{ }

TimefieldRenderCommand::TimefieldRenderCommand(const Column InColumn, const Time InTime, const std::function<void(sf::RenderTarget* const, const TimefieldMetrics&, int, int)>& InRenderWork)
    : ColumnPoint(InColumn), TimePoint(InTime), RenderWork(InRenderWork)
{ }