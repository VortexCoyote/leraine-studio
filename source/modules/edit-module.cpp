#include "edit-module.h"

bool EditModule::OnMouseLeftButtonReleased()
{
	return _EditModes[_SelectedEditMode]->OnMouseLeftButtonReleased();
}

bool EditModule::OnMouseRightButtonClicked(const bool InIsShiftDown)
{
	return _EditModes[_SelectedEditMode]->OnMouseRightButtonClicked(InIsShiftDown);
}

bool EditModule::OnMouseRightButtonReleased()
{
	return _EditModes[_SelectedEditMode]->OnMouseRightButtonReleased();
}

bool EditModule::OnMouseDrag()
{
	return _EditModes[_SelectedEditMode]->OnMouseDrag();
}

bool EditModule::OnMouseLeftButtonClicked(const bool InIsShiftDown)
{
	return _EditModes[_SelectedEditMode]->OnMouseLeftButtonClicked(InIsShiftDown);
}

void EditModule::SubmitToRenderGraph(TimefieldRenderGraph& InOutTimefieldRenderGraph)
{
	_EditModes[_SelectedEditMode]->SubmitToRenderGraph(InOutTimefieldRenderGraph);
}

void EditModule::SetChart(Chart* const InOutChart)
{
	EditMode::SetChart(InOutChart);
}

void EditModule::SetCursorData(const Cursor& InCursor)
{
	EditMode::SetCursorData(InCursor);
}

bool EditModule::Tick(const float& InDeltaTime) 
{
	_EditModes[_SelectedEditMode]->Tick();

	return true;
}
