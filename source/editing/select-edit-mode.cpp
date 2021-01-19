#include "select-edit-mode.h"

#include <SFML/Graphics.hpp>

bool SelectEditMode::OnMouseLeftButtonReleased()
{
    _IsAreaSelecting = false;

    if(!(static_Cursor.TimefieldSide != _AnchoredCursor.TimefieldSide || static_Cursor.TimefieldSide == Cursor::FieldPosition::Middle && _AnchoredCursor.TimefieldSide == Cursor::FieldPosition::Middle))
        return false;

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

bool SelectEditMode::OnCopy() 
{
    return false;
}

bool SelectEditMode::OnPaste() 
{
    //I hate parsing strings in c++ 

    _PastePreviewNotes.clear();
    _LowestPasteTimePoint = INT_MAX;

    _IsPreviewingPaste = true;

    std::string parsableNotes = sf::Clipboard::getString();

    std::string time;
    std::string column;

    bool leftParenthesis = false;
    bool rightParenthesis = false;

    for (size_t letterIndex = 0; letterIndex < parsableNotes.size(); ++letterIndex)
    {
        char letter = parsableNotes[letterIndex];

        if(leftParenthesis && !rightParenthesis)
        {
            if(letter == '|')
            {
                letterIndex++;
                 
                column += parsableNotes[letterIndex];

                Note note;
                note.BeatSnap = -1;
                note.TimePoint = std::stoi(time);
                note.Type = Note::EType::Common;

                _LowestPasteTimePoint = std::min(_LowestPasteTimePoint, std::stoi(time));
                _PastePreviewNotes.push_back({ std::stoi(column), note});

                column = "";
                time = "";

                letterIndex++;
                letterIndex++;

                letter = parsableNotes[letterIndex];
            }

            time += letter;
        }

        if(letter == '(')
            leftParenthesis = true;
    
        if(letter == ')')
            rightParenthesis = true;
    }

    return false;
}

void SelectEditMode::OnReset() 
{
    _PastePreviewNotes.reserve(100);

    _IsPreviewingPaste = false;
    _IsAreaSelecting = false;

    _SelectedNotes.clear();
    _PastePreviewNotes.clear();
}


bool SelectEditMode::OnMouseLeftButtonClicked(const bool InIsShiftDown)
{
    _AnchoredCursor = static_Cursor;
    _IsAreaSelecting = true;

    _SelectedNotes.clear();

    if(_IsPreviewingPaste)
    {
        for(auto& [column, note] : _PastePreviewNotes)
            note.TimePoint = static_Cursor.TimePoint + note.TimePoint - _LowestPasteTimePoint;

        static_Chart->BulkPlaceNotes(_PastePreviewNotes);
        _IsPreviewingPaste = false;
    }

    return true;
}

void SelectEditMode::SubmitToRenderGraph(TimefieldRenderGraph& InOutTimefieldRenderGraph)
{
    if(_IsPreviewingPaste)
    {
        for(const auto [column, note] : _PastePreviewNotes)
            InOutTimefieldRenderGraph.SubmitCommonNoteRenderCommand(column, static_Cursor.TimePoint + note.TimePoint - _LowestPasteTimePoint, note.BeatSnap, 128);

        return;
    }

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