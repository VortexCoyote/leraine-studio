#pragma once

#include "base/edit-mode.h"

class BpmEditMode : public EditMode
{
public:

	bool OnMouseLeftButtonClicked(const bool InIsShiftDown) override;
	bool OnMouseLeftButtonReleased() override;

	bool OnMouseRightButtonClicked(const bool InIsShiftDown) override;

	void SubmitToRenderGraph(TimefieldRenderGraph& InOutTimefieldRenderGraph, const Time InTimeBegin, const Time InTimeEnd) override;
	void Tick() override;

private:

	void PlaceAutoTimePoint();
	void PlaceTimePoint();

	void DisplayBpmNode(BpmPoint& InBpmPoint, const int InScreenX, const int InScreenY, const bool InIsPinned = false);

	std::vector<BpmPoint*>* _VisibleBpmPoints = nullptr;
	BpmPoint* _HoveredBpmPoint = nullptr;
	BpmPoint* _MovableBpmPoint = nullptr;

	BpmPoint _MovableBpmPointInitialValue;
	
	BpmPoint* _PreviousBpmPoint = nullptr;
	BpmPoint* _NextBpmPoint = nullptr;

	BpmPoint* _PinnedBpmPoint = nullptr;
};