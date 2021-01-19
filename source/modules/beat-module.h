#pragma once

#include <functional>
#include <set>

#include "base/module.h"

/*
* this module is responsible for a number of sleepless nights, please proceed with caution
*/

struct BeatLine
{
	Time TimePoint = 0;

	int BeatCount = 0;
	int BeatDivision = 0;
	int BeatSnap = -1;
};

class BeatModule : public Module
{
public:

	virtual bool StartUp() override;

public:

	void AssignSnapsToNotesInChart(Chart* const InChart);
	void AssignSnapsToNotesInTimeSlice(Chart* const InChart, TimeSlice& InOutTimeSlice, const bool InResnapNotes = false);
	void GenerateTimeRangeBeatLines(const Time InTimeBegin, const Time InTimeEnd, Chart* const InChart, const int InBeatDivision, const bool InSkipClearCollection = false);
	void IterateThroughBeatlines(std::function<void(const BeatLine&)> InWork);
	
	int GetBeatSnap(const BeatLine& InBeatLine, const int InBeatDivision);
	int GetBeatSnap(const int InBeatCount, const int InBeatDivision);

	BeatLine GetNextBeatLine(const Time InTime);
	BeatLine GetPreviousBeatLine(const Time InTime);
	BeatLine GetCurrentBeatLine(const Time InTime, const Time InBias = 0, const bool InScanDown = true);

	const int GetNextSnap(const int InCurrentSnap);
	const int GetPreviousSnap(const int InCurrentSnap);

private:

	bool IsBeatThisDivision(const int InBeatCount, const int InBeatDivision, const int InDenominator);

	std::vector<BeatLine> _OnFieldBeatLines;
	
	std::set<int> _LegalSnaps;
};