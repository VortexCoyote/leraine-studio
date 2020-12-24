#include "select-edit-mode.h"

#include <SFML/Graphics.hpp>

bool SelectEditMode::OnMouseLeftButtonReleased()
{
    _IsAreaSelecting = false;

    const Time timeBegin = std::min(_AnchoredCursor.UnsnappedTimePoint, static_Cursor.UnsnappedTimePoint);
    const Time timeEnd   = std::max(_AnchoredCursor.UnsnappedTimePoint, static_Cursor.UnsnappedTimePoint);

    const Column columnMin = std::min(_AnchoredCursor.Column, static_Cursor.Column);
    const Column columnMax = std::max(_AnchoredCursor.Column, static_Cursor.Column);

    static_Chart->IterateNotesInTimeRange(timeBegin, timeEnd, [this, &timeBegin, &timeEnd, &columnMin, &columnMax](Note& InOutNote, const Column& InColumn)
    {
        if((InOutNote.Type == Note::EType::Common || InOutNote.Type == Note::EType::HoldBegin) && (InColumn >= columnMin && InColumn <= columnMax))
            _SelectedNotes[InColumn].insert(&InOutNote);
    });

    return true;
}

void SelectEditMode::Tick() 
{
    
}

void SelectEditMode::OnReset() 
{
    _IsAreaSelecting = false;
    _SelectedNotes.clear();
}


bool SelectEditMode::OnMouseLeftButtonClicked(const bool InIsShiftDown)
{
    _AnchoredCursor = static_Cursor;
    _IsAreaSelecting = true;

    _SelectedNotes.clear();

    return true;
}

void SelectEditMode::SubmitToRenderGraph(TimefieldRenderGraph& InOutTimefieldRenderGraph)
{
    for(const auto [column, noteCollection] : _SelectedNotes)
    {
        for(auto selectedNote : noteCollection)
        {
            InOutTimefieldRenderGraph.SubmitTimefieldRenderCommand(column, selectedNote->TimePoint,
            [](sf::RenderTarget* const InRenderTarget, const TimefieldMetrics& InTimefieldMetrics, const int InScreenX, const int InScreenY)
            {
                sf::RectangleShape rectangle;

                rectangle.setPosition(InScreenX, InScreenY - InTimefieldMetrics.NoteScreenPivot);
                rectangle.setSize(sf::Vector2f(InTimefieldMetrics.ColumnSize, InTimefieldMetrics.ColumnSize));

                rectangle.setFillColor(sf::Color(255, 255, 255, 64));
                rectangle.setOutlineColor(sf::Color(255, 255, 255, 255));
                rectangle.setOutlineThickness(1.0f);

                InRenderTarget->draw(rectangle);
            }); 
        }
    }

    if(!_IsAreaSelecting)
        return;

    InOutTimefieldRenderGraph.SubmitTimefieldRenderCommand(_AnchoredCursor.Column, _AnchoredCursor.UnsnappedTimePoint, 
    [this](sf::RenderTarget* const InRenderTarget, const TimefieldMetrics& InTimefieldMetrics, const int InScreenX, const int InScreenY)
    {
        sf::RectangleShape rectangle;
        
        rectangle.setPosition(_AnchoredCursor.X, InScreenY);
        rectangle.setSize(sf::Vector2f(static_Cursor.X - _AnchoredCursor.X, static_Cursor.Y - InScreenY));

        rectangle.setFillColor(sf::Color(255, 255, 196, 64));
        rectangle.setOutlineColor(sf::Color(255, 255, 196, 96));
        rectangle.setOutlineThickness(1.0f);

        InRenderTarget->draw(rectangle);
    });
}