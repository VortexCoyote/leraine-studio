#pragma once

#include "chart.h"

struct Cursor
{
	enum class FieldPosition
	{
		Left,
		Middle,
		Right,

		COUNT
	} TimefieldSide;

	Time TimePoint = 0;
	Time UnsnappedTimePoint = 0;
	Column CursorColumn = 0;

	std::vector<const Note*> HoveredNotes;

	int BeatSnap = -1;
	int TimeFieldY = 0;

	int X = 0;
	int Y = 0;
};