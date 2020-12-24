#pragma once

struct TimefieldMetrics
{
public: //constants

	int ColumnSize = 64;

	int NoteScreenPivotsLookup[16] = { 0, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	int HitLine = 196;
	int KeyAmount = 0;
	int SideSpace = 32;
	
public: //dynamics
	
	int NoteScreenPivot;
	int LeftSidePositionFromCenter;
	int LeftSidePosition;
	int FirstColumnPosition;
	int FieldWidth;
	int FieldWidthHalf;
	int NoteFieldWidth;
	int NoteFieldWidthHalf;
	int HitLinePosition;
};