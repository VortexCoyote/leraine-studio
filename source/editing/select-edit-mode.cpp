#include "select-edit-mode.h"

#include <SFML/Graphics.hpp>
#include <climits>
#include <sstream>
#include <cmath>

#include "imgui.h"


bool SelectEditMode::OnCopy() 
{
    std::string clipboard =  "LeraineStudio:";

    for(const auto [column, noteCollection] : _SelectedNotes.Notes)
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

    PUSH_NOTIFICATION("Copied %d Notes", _SelectedNotes.NoteAmount);
    sf::Clipboard::setString(clipboard);

    return true;
}

bool SelectEditMode::OnPaste() 
{
    //I hate parsing strings in c++ 

    _MostRightColumn = 0;
    _MostLeftColumn = static_Chart->KeyAmount - 1;

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

            _MostRightColumn = std::max(_MostRightColumn, column);
            _MostLeftColumn = std::min(_MostLeftColumn, column);
        }
    }

    return true;
}

bool SelectEditMode::OnMirror() 
{
    if(_IsPreviewingPaste)
    {
        static_Chart->MirrorNotes(_PastePreviewNotes);
        PUSH_NOTIFICATION("Mirrored %d Paste Notes", _PastePreviewNotes.size());
        
        return true;
    }
    
    if(_SelectedNotes.HasNotes)
    {
        PUSH_NOTIFICATION("Mirrored %d Notes", _SelectedNotes.NoteAmount);
        static_Chart->MirrorNotes(_SelectedNotes);
    }

    return true;
}

bool SelectEditMode::OnDelete() 
{
    if(_SelectedNotes.HasNotes)
    {
        PUSH_NOTIFICATION("Deleted %d Notes", _SelectedNotes.NoteAmount);
        static_Chart->BulkRemoveNotes(_SelectedNotes);
    }

    return true;
}

bool SelectEditMode::OnSelectAll() 
{
    static_Chart->FillNoteCollectionWithAllNotes(_SelectedNotes);
    PUSH_NOTIFICATION("Selected %d Notes", _SelectedNotes.NoteAmount);

    return true;
}

void SelectEditMode::OnReset() 
{
    _PastePreviewNotes.reserve(100);

    _IsPreviewingPaste = false;
    _IsAreaSelecting = false;

    _SelectedNotes.Clear();
    _PastePreviewNotes.clear();
}

void SelectEditMode::Tick() 
{
    if(_IsMovingNote)
        return;

    if(!static_Cursor.HoveredNotes.empty() && static_Cursor.TimefieldSide == Cursor::FieldPosition::Middle && !_IsAreaSelecting)
    {
        _HoveredNoteColumn = static_Cursor.CursorColumn;
        _HoveredNote = static_Chart->FindNote(static_Cursor.HoveredNotes.back()->TimePoint, _HoveredNoteColumn);
    }
    else
        _HoveredNote = nullptr;
}

bool SelectEditMode::OnMouseLeftButtonClicked(const bool InIsShiftDown)
{
    _AnchoredCursor = static_Cursor;

    if(_HoveredNote != nullptr)
    {
        if(_SelectedNotes.NoteAmount > 1)
        {
            Time smallestDis = INT32_MAX;
            for(auto& [column, notes] : _SelectedNotes.Notes)
                for(auto& note : notes)
                {
                    Note noteCopy = *note;
                    noteCopy.BeatSnap = -1;

                    _MostRightColumn = std::max(_MostRightColumn, column);
                    _MostLeftColumn = std::min(_MostLeftColumn, column);

                    _PastePreviewNotes.push_back({column, noteCopy});
                    _DraggingNotes.PushNote(column, note);

                    if(abs(static_Cursor.UnsnappedTimePoint - note->TimePoint) < smallestDis)
                    {
                        smallestDis = abs(static_Cursor.UnsnappedTimePoint - note->TimePoint);
                        _LowestPasteTimePoint = note->TimePoint;
                    }
                }
        }
    
        _SelectedNotes.Clear();
        
        _DraggingNote = _HoveredNote;
        return _IsMovingNote = true;
    }

    _SelectedNotes.Clear();  
    
    _IsAreaSelecting = true;

    if(!_IsPreviewingPaste)
        return true;
    
    SetNewPreviewPasteLocation();

    PUSH_NOTIFICATION("Placed %d Notes", _PastePreviewNotes.size());

    static_Chart->BulkPlaceNotes(_PastePreviewNotes);
    _IsPreviewingPaste = false;
    _PastePreviewNotes.clear();

    return true;
}

bool SelectEditMode::OnMouseLeftButtonReleased()
{
    if(_IsMovingNote)
    {
        if(_PastePreviewNotes.size() > 0)
        {   
            SetNewPreviewPasteLocation();
            
            Time minTimePoint = (_HighestPasteTimepoint < _DraggingNotes.MaxTimePoint) ? _HighestPasteTimepoint - (_DraggingNotes.MaxTimePoint - _DraggingNotes.MinTimePoint) : _DraggingNotes.MinTimePoint;
            Time maxTimePoint = (_HighestPasteTimepoint < _DraggingNotes.MaxTimePoint) ? _DraggingNotes.MaxTimePoint : _HighestPasteTimepoint;
            
            static_Chart->RegisterTimeSliceHistoryRanged(minTimePoint - TIMESLICE_LENGTH, maxTimePoint + TIMESLICE_LENGTH);
            static_Chart->BulkRemoveNotes(_DraggingNotes, true);
            static_Chart->BulkPlaceNotes(_PastePreviewNotes, true);

            PUSH_NOTIFICATION("Moved %d Notes", _PastePreviewNotes.size());

            _LowestPasteTimePoint = INT32_MAX;
            _HighestPasteTimepoint = INT32_MIN;

            _MostRightColumn = 0;
            _MostLeftColumn = static_Chart->KeyAmount - 1;

            _PastePreviewNotes.clear();
            
            return _IsMovingNote = false;
        }

        if (_DraggingNote->Type == Note::EType::HoldEnd && _DraggingNote->TimePointBegin >= static_Cursor.TimePoint)
        {
            // TODO : Make the one-undo implementation
            Time TimePointBegin = _DraggingNote->TimePointBegin;
            static_Chart->RemoveNote(_DraggingNote->TimePointBegin, static_Cursor.CursorColumn);
            static_Chart->PlaceNote(TimePointBegin, static_Cursor.CursorColumn, static_Cursor.BeatSnap);

            return _IsMovingNote = false;
        }

        if (_DraggingNote->Type == Note::EType::HoldBegin && _DraggingNote->TimePointEnd <= static_Cursor.TimePoint)
        {
            // TODO : Make the one-undo implementation
            Time TimePointEnd = _DraggingNote->TimePointEnd;
            static_Chart->RemoveNote(_DraggingNote->TimePointEnd, static_Cursor.CursorColumn);
            static_Chart->PlaceNote(TimePointEnd, static_Cursor.CursorColumn, static_Cursor.BeatSnap);

            return _IsMovingNote = false;
        }

        _HoveredNote = static_Chart->MoveNote(_HoveredNote->TimePoint, static_Cursor.TimePoint, _HoveredNoteColumn, static_Cursor.CursorColumn, static_Cursor.BeatSnap);
        _HoveredNoteColumn = static_Cursor.CursorColumn;

        return _IsMovingNote = false;
    } 

    if(!(static_Cursor.TimefieldSide != _AnchoredCursor.TimefieldSide || static_Cursor.TimefieldSide == Cursor::FieldPosition::Middle && _AnchoredCursor.TimefieldSide == Cursor::FieldPosition::Middle))
        return _IsAreaSelecting = false;

    if(!_IsAreaSelecting)
        return false;

    _IsAreaSelecting = false;
    
    const Time timeBegin = std::min(_AnchoredCursor.UnsnappedTimePoint, static_Cursor.UnsnappedTimePoint);
    const Time timeEnd   = std::max(_AnchoredCursor.UnsnappedTimePoint, static_Cursor.UnsnappedTimePoint);

    const Column columnMin = std::min(_AnchoredCursor.CursorColumn, static_Cursor.CursorColumn);
    const Column columnMax = std::max(_AnchoredCursor.CursorColumn, static_Cursor.CursorColumn);

    static_Chart->IterateNotesInTimeRange(timeBegin, timeEnd, [this, &timeBegin, &timeEnd, &columnMin, &columnMax](Note& InOutNote, const Column& InColumn)
    {
        if((InOutNote.Type == Note::EType::Common || InOutNote.Type == Note::EType::HoldBegin) && (InColumn >= columnMin && InColumn <= columnMax))
            _SelectedNotes.PushNote(InColumn, &InOutNote);
    });

    if(_SelectedNotes.NoteAmount != 0)
        PUSH_NOTIFICATION("Selected %d Notes", _SelectedNotes.NoteAmount);

    return true;
}

void SelectEditMode::SubmitToRenderGraph(TimefieldRenderGraph& InOutTimefieldRenderGraph, const Time InTimeBegin, const Time InTimeEnd)
{
    if(_PastePreviewNotes.size() != 0 || _IsPreviewingPaste)
    {
        for(auto& [column, note] : _PastePreviewNotes)
        {
            Column newColumn = column + GetDelteColumn();

            switch(note.Type)
            {
                case Note::EType::Common:
                    InOutTimefieldRenderGraph.SubmitCommonNoteRenderCommand(newColumn, static_Cursor.TimePoint + note.TimePoint - _LowestPasteTimePoint, note.BeatSnap, 128);
                break;

                case Note::EType::HoldBegin:
                    InOutTimefieldRenderGraph.SubmitHoldNoteRenderCommand(newColumn, static_Cursor.TimePoint + note.TimePointBegin - _LowestPasteTimePoint, 
                                                                                     static_Cursor.TimePoint + note.TimePointEnd   - _LowestPasteTimePoint, -1, -1, 128);
                break;
            }
        }
        return;
    }

    for(const auto& [column, noteCollection] : _SelectedNotes.Notes)
    {
        for(auto& selectedNote : noteCollection)
        {
            if(selectedNote->TimePoint < InTimeBegin - TIMESLICE_LENGTH || selectedNote->TimePoint > InTimeEnd + TIMESLICE_LENGTH)
                continue;

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

    if(_SelectedNotes.HasNotes && static_Flags.ShowColumnHeatmap)
    {
         for(const auto& [column, count] : _SelectedNotes.ColumnNoteCount)
         {
             sf::Uint8 alpha = sf::Uint8(std::pow(float(count) / float(_SelectedNotes.HighestColumnAmount), 1.f) * 255.f);
            
            //TODO: rendercommand for multiple timepoints and columns since this is extremely hacky
            int* minY = new int();

            InOutTimefieldRenderGraph.SubmitTimefieldRenderCommand(column, _SelectedNotes.MinTimePoint ,
            [minY](sf::RenderTarget* const InRenderTarget, const TimefieldMetrics& InTimefieldMetrics, const int InScreenX, const int InScreenY)
            { *minY = InScreenY; });

            InOutTimefieldRenderGraph.SubmitTimefieldRenderCommand(column, _SelectedNotes.MaxTimePoint,
            [alpha, minY](sf::RenderTarget* const InRenderTarget, const TimefieldMetrics& InTimefieldMetrics, const int InScreenX, const int InScreenY)
            {
                sf::RectangleShape rectangle;

                rectangle.setPosition(InScreenX, InScreenY - InTimefieldMetrics.NoteScreenPivot);
                rectangle.setSize(sf::Vector2f(InTimefieldMetrics.ColumnSize, *minY - InScreenY + InTimefieldMetrics.ColumnSize));

                rectangle.setFillColor(sf::Color(alpha, 255 - alpha, 0, 128));
                rectangle.setOutlineThickness(0.0f);

                InRenderTarget->draw(rectangle);

                delete minY;
            }); 
         }
    }

    if(_HoveredNote || _IsMovingNote)
    {
        Column column = _HoveredNoteColumn;
        Time timePoint = _HoveredNote->TimePoint;

        if(_IsMovingNote)
        {
            column = static_Cursor.CursorColumn;
            timePoint = static_Cursor.TimePoint;

            switch (_DraggingNote->Type)
            {
            case Note::EType::HoldBegin:
                InOutTimefieldRenderGraph.SubmitHoldNoteRenderCommand(column, timePoint, _DraggingNote->TimePointEnd, -1, -1, 128);
                break;

            case Note::EType::HoldEnd:
                InOutTimefieldRenderGraph.SubmitHoldNoteRenderCommand(column, _DraggingNote->TimePointBegin, timePoint, -1, -1, 128);
                break;
            
            default:
                InOutTimefieldRenderGraph.SubmitCommonNoteRenderCommand(column, timePoint, -1, 128);
                break;
            }

        }
        else
        {
            InOutTimefieldRenderGraph.SubmitTimefieldRenderCommand(column, timePoint,
            [](sf::RenderTarget* const InRenderTarget, const TimefieldMetrics& InTimefieldMetrics, const int InScreenX, const int InScreenY)
            {
                sf::RectangleShape rectangle;

                rectangle.setPosition(InScreenX, InScreenY - InTimefieldMetrics.NoteScreenPivot);
                rectangle.setSize(sf::Vector2f(InTimefieldMetrics.ColumnSize, InTimefieldMetrics.ColumnSize));

                rectangle.setFillColor(sf::Color(64, 255, 64, 64));
                rectangle.setOutlineColor(sf::Color(64, 255, 64, 255));
                rectangle.setOutlineThickness(1.0f);

                InRenderTarget->draw(rectangle);
            }); 
        }
    }

    if(!_IsAreaSelecting)
        return;

    InOutTimefieldRenderGraph.SubmitTimefieldRenderCommand(_AnchoredCursor.CursorColumn, _AnchoredCursor.UnsnappedTimePoint, 
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

int SelectEditMode::GetDelteColumn() 
{
    int deltaColumn = int(static_Cursor.CursorColumn) - int(_MostLeftColumn);
    int keyAmount = static_Chart->KeyAmount - 1;

    if(int(_MostRightColumn) + deltaColumn > keyAmount)
        deltaColumn = deltaColumn - (int(_MostRightColumn) + deltaColumn - keyAmount);

    if(int(_MostLeftColumn) + deltaColumn < 0)
        deltaColumn = deltaColumn - (int(_MostLeftColumn) + deltaColumn);

    return deltaColumn;
}

void SelectEditMode::SetNewPreviewPasteLocation() 
{
    for(auto& [column, note] : _PastePreviewNotes)
    {
        Column newColumn = column + GetDelteColumn();

        note.TimePoint = static_Cursor.TimePoint + note.TimePoint - _LowestPasteTimePoint;
        note.TimePointBegin = static_Cursor.TimePoint + note.TimePointBegin - _LowestPasteTimePoint;
        note.TimePointEnd = static_Cursor.TimePoint + note.TimePointEnd - _LowestPasteTimePoint;

        column = newColumn;

        _HighestPasteTimepoint = std::max(_HighestPasteTimepoint, note.TimePoint);
    }
}