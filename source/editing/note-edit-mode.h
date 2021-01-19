#pragma once

#include "base/edit-mode.h"

class NoteEditMode : public EditMode
{
public:

	bool OnMouseLeftButtonClicked(const bool InIsShiftDown) override;
	bool OnMouseLeftButtonReleased() override;

	bool OnMouseRightButtonClicked(const bool InIsShiftDown) override;

	void SubmitToRenderGraph(TimefieldRenderGraph& InOutTimefieldRenderGraph) override;
	
private:

	Cursor _AnchoredHoldCursor;

	bool _IsPlacingHold = false;
};