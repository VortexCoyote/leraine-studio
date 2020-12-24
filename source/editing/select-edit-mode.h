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

	void Tick() override;

	void OnReset() override;
	void SubmitToRenderGraph(TimefieldRenderGraph& InOutTimefieldRenderGraph) override;

private:

	std::unordered_map<Column, std::unordered_set<Note*>> _SelectedNotes;

	Cursor _AnchoredCursor;
	bool _IsAreaSelecting = false;
};