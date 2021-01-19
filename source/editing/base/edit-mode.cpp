#include "edit-mode.h"

Chart* EditMode::static_Chart = nullptr;
Cursor EditMode::static_Cursor;

void EditMode::SetChart(Chart* const InOutChart)
{
	static_Chart = InOutChart;
}

void EditMode::SetCursorData(const Cursor& InCursor)
{
	static_Cursor = InCursor;
}

bool EditMode::OnMouseLeftButtonClicked(const bool InIsShiftDown)
{
	return false;
}

bool EditMode::OnMouseLeftButtonReleased()
{
	return false;
}

bool EditMode::OnMouseRightButtonClicked(const bool InIsShiftDown)
{
	return false;
}

bool EditMode::OnMouseRightButtonReleased()
{
	return false;
}

bool EditMode::OnMouseDrag()
{
	return false;
}

bool EditMode::OnCopy() 
{
	return false;
}

bool EditMode::OnPaste() 
{
	return false;
}

void EditMode::OnReset() 
{

}

void EditMode::SubmitToRenderGraph(TimefieldRenderGraph& InOutTimefieldRenderGraph)
{
	
}

void EditMode::Tick() 
{
	
}
