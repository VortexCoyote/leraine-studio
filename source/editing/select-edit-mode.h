#pragma once

#include "base/edit-mode.h"

#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <climits>

/*
* this edit-mode is one giant edgecase 
*/
class SelectEditMode : public EditMode
{
public:

	bool OnMouseLeftButtonClicked(const bool InIsShiftDown) override;
	bool OnMouseLeftButtonReleased() override;

	bool OnCopy() override;
	bool OnPaste() override;
	bool OnMirror() override;
	bool OnDelete() override;
	bool OnSelectAll() override;

	void OnReset() override;
	void Tick() override;
	void SubmitToRenderGraph(TimefieldRenderGraph& InOutTimefieldRenderGraph, const Time InTimeBegin, const Time InTimeEnd) override;

private:

	int GetDelteColumn();
	void SetNewPreviewPasteLocation();

	NoteReferenceCollection _SelectedNotes;
	NoteReferenceCollection _DraggingNotes;
	std::vector<std::pair<Column, Note>> _PastePreviewNotes;

	Time _LowestPasteTimePoint = INT_MAX;
	Time _HighestPasteTimepoint = INT_MIN;
	Cursor _AnchoredCursor;

	bool _IsAreaSelecting = false;
	bool _IsPreviewingPaste = false;
	bool _IsMovingNote = false;

	Column _MostRightColumn = 0;
	Column _MostLeftColumn = 0;

	Note* _DraggingNote = nullptr;
	Note* _HoveredNote = nullptr;
	Column _HoveredNoteColumn = 0;
};