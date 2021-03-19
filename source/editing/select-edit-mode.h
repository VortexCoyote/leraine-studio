#pragma once

#include "base/edit-mode.h"

#include <unordered_set>
#include <unordered_map>
#include <utility>

class SelectEditMode : public EditMode
{
public:

	bool OnMouseLeftButtonClicked(const bool InIsShiftDown) override;
	bool OnMouseLeftButtonReleased() override;

	bool OnCopy() override;
	bool OnPaste() override;

	void OnReset() override;
	void SubmitToRenderGraph(TimefieldRenderGraph& InOutTimefieldRenderGraph, const Time InTimeBegin, const Time InTimeEnd) override;

private:

	std::unordered_map<Column, std::unordered_set<Note*>> _SelectedNotes;
	std::vector<std::pair<Column, Note>> _PastePreviewNotes;

	Time _LowestPasteTimePoint = INT_MAX;
	Cursor _AnchoredCursor;

	bool _IsAreaSelecting = false;
	bool _IsPreviewingPaste = false;

	Column _MostRightColumn = 0;
	Column _MostLeftColumn = 0;
};