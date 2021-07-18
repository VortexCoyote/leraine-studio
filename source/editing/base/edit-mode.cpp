#include "edit-mode.h"

Chart* EditMode::static_Chart = nullptr;
Cursor EditMode::static_Cursor;
EditFlags EditMode::static_Flags;

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

bool EditMode::OnMirror() 
{
	return false;
}

bool EditMode::OnDelete() 
{
	return false;
}

bool EditMode::OnSelectAll() 
{
	return false;
}

void EditMode::OnReset() 
{

}

void EditMode::SubmitToRenderGraph(TimefieldRenderGraph& InOutTimefieldRenderGraph, const Time InTimeBegin, const Time InTimeEnd)
{
	
}

void EditMode::Tick() 
{
	
}
