#include "note-edit-mode.h"

bool NoteEditMode::OnMouseRightButtonClicked(const bool InIsShiftDown)
{
    if(static_Cursor.HoveredNotes.size() != 0)
    {
        const Note& firstNote = *(static_Cursor.HoveredNotes.back());
        static_Chart->RemoveNote(firstNote.TimePoint, static_Cursor.CursorColumn);
    }

    return false;
}

bool NoteEditMode::OnMouseLeftButtonClicked(const bool InIsShiftDown)
{
    if(static_Cursor.TimefieldSide != Cursor::FieldPosition::Middle)
        return false;

    if(InIsShiftDown)
    {
        _AnchoredHoldCursor = static_Cursor;
        _IsPlacingHold = true;
    }
    else
    {
        return static_Chart->PlaceNote(static_Cursor.TimePoint, static_Cursor.CursorColumn);
    }
    
    return false;
}

bool NoteEditMode::OnMouseLeftButtonReleased()
{   
    if(_IsPlacingHold)
    {
        _IsPlacingHold = false;

        if(_AnchoredHoldCursor.TimePoint < static_Cursor.TimePoint)
            return static_Chart->PlaceHold(_AnchoredHoldCursor.TimePoint, static_Cursor.TimePoint, _AnchoredHoldCursor.CursorColumn);
        else
            return static_Chart->PlaceHold(_AnchoredHoldCursor.TimePoint, _AnchoredHoldCursor.TimePoint, _AnchoredHoldCursor.CursorColumn);
    }

    return false;
}

void NoteEditMode::SubmitToRenderGraph(TimefieldRenderGraph& InOutTimefieldRenderGraph, const Time InTimeBegin, const Time InTimeEnd)
{
    if(_IsPlacingHold)
        if(_AnchoredHoldCursor.TimePoint < static_Cursor.TimePoint)
            InOutTimefieldRenderGraph.SubmitHoldNoteRenderCommand(_AnchoredHoldCursor.CursorColumn, _AnchoredHoldCursor.TimePoint, static_Cursor.TimePoint, _AnchoredHoldCursor.BeatSnap, static_Cursor.BeatSnap, 156);
        else
            InOutTimefieldRenderGraph.SubmitCommonNoteRenderCommand(_AnchoredHoldCursor.CursorColumn, _AnchoredHoldCursor.TimePoint, _AnchoredHoldCursor.BeatSnap, 156);
    else if(static_Cursor.TimefieldSide == Cursor::FieldPosition::Middle)
        InOutTimefieldRenderGraph.SubmitCommonNoteRenderCommand(static_Cursor.CursorColumn, static_Cursor.TimePoint, static_Cursor.BeatSnap, 156);
}