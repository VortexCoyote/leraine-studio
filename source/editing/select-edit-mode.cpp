#include "select-edit-mode.h"

#include <SFML/Graphics.hpp>
#include <sstream>

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
    std::string clipboard =  "LeraineStudio:";

    for(const auto [column, noteCollection] : _SelectedNotes)
    {
        for(auto selectedNote : noteCollection)
        {
            std::string noteSegment = "";
            switch(selectedNote->Type)
            {
                case Note::EType::Common:
                {
                    noteSegment += "N";
                    noteSegment += std::to_string(column) + "|";
                    noteSegment += std::to_string(selectedNote->TimePoint) + ";";
                }
                break;

                case Note::EType::HoldBegin:
                {
                    noteSegment += "H";
                    noteSegment += std::to_string(column) + "|";
                    noteSegment += std::to_string(selectedNote->TimePointBegin) + ",";
                    noteSegment += std::to_string(selectedNote->TimePointEnd) + ";";
                }
                break;
            }

            clipboard += noteSegment;
        }
    }

    sf::Clipboard::setString(clipboard);

    return true;
}

bool SelectEditMode::OnPaste() 
{
    //I hate parsing strings in c++ 

    _PastePreviewNotes.clear();
    _LowestPasteTimePoint = INT_MAX;

    _IsPreviewingPaste = true;

    std::string clipboard = sf::Clipboard::getString();

    std::string signature = "";

    for (auto character : clipboard)
    {
        if(character == ':')
            break;

        signature += character;
    }

    if(signature == "LeraineStudio")
    {
        clipboard.erase(0, std::string("LeraineStudio:").size());

        for(size_t index = 0; index < clipboard.size(); ++index)
        {
            char character = clipboard[index];

            Note note;
            Column column;

            switch (character)
            {
            case 'N':
            {
                character = clipboard[++index];

                std::string parsedColumn;
                parsedColumn += character;
                column = std::stoi(parsedColumn);

                character = clipboard[++index];

                std::string parsedTimePoint = "";
                while(character != ';')
                    character = clipboard[++index], parsedTimePoint += character;

                note.Type = Note::EType::Common;
                note.TimePoint = std::stoi(parsedTimePoint);

                _LowestPasteTimePoint = std::min( _LowestPasteTimePoint, note.TimePoint);
                _PastePreviewNotes.push_back({column, note});
            }
            break;

            case 'H':
            {
                character = clipboard[++index];

                std::string parsedColumn;
                parsedColumn += character;
                column = std::stoi(parsedColumn);

                character = clipboard[++index];
                character = clipboard[++index];

                std::string parsedTimePointBegin = "";
                std::string parsedTimePointEnd = "";
                
                bool endParse = false;

                while(character != ';')
                {
                    if(character == ',')
                        endParse = true, character = clipboard[++index];

                    if(!endParse)
                        parsedTimePointBegin += character;
                    else
                        parsedTimePointEnd += character;

                    character = clipboard[++index];
                }

                note.Type = Note::EType::HoldBegin;
                note.TimePoint = std::stoi(parsedTimePointBegin);
                note.TimePointBegin = std::stoi(parsedTimePointBegin);
                note.TimePointEnd = std::stoi(parsedTimePointEnd);

                _LowestPasteTimePoint = std::min( _LowestPasteTimePoint, note.TimePoint);
                _PastePreviewNotes.push_back({column, note});
            }
            break;
            
            default:
                break;
            }
        }
    }

    return true;
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
        {
            note.TimePoint = static_Cursor.TimePoint + note.TimePoint - _LowestPasteTimePoint;
            note.TimePointBegin = static_Cursor.TimePoint + note.TimePointBegin - _LowestPasteTimePoint;
            note.TimePointEnd = static_Cursor.TimePoint + note.TimePointEnd - _LowestPasteTimePoint;
        }

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
        {
            switch(note.Type)
            {
                case Note::EType::Common:
                    InOutTimefieldRenderGraph.SubmitCommonNoteRenderCommand(column, static_Cursor.TimePoint + note.TimePoint - _LowestPasteTimePoint, note.BeatSnap, 128);
                break;

                case Note::EType::HoldBegin:
                    InOutTimefieldRenderGraph.SubmitHoldNoteRenderCommand(column, static_Cursor.TimePoint + note.TimePointBegin - _LowestPasteTimePoint, 
                                                                                  static_Cursor.TimePoint + note.TimePointEnd   - _LowestPasteTimePoint, -1, -1, 128);
                break;
            }
        }
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