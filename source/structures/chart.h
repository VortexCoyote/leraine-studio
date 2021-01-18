#pragma once

#include <map>
#include <vector>
#include <stack>
#include <map>
#include <string>
#include <utility>
#include <functional>

/*
* these types should not have any dependencies on any systems or modules.
* the chart should almost be treated as a datastructure, since its only handling its own data.
* worth to note is that this datastructure makes assumptions in which type certain metrics are.
*/

#define TIMESLICE_LENGTH 500

typedef int Time;
typedef size_t Column;

struct Note
{
	enum class EType
	{
		Common,
		HoldBegin,
		HoldIntermediate,
		HoldEnd,

		COUNT
	} Type;

	Time TimePoint;
	int BeatSnap = -1;

	Time TimePointBegin = 0;
	Time TimePointEnd = 0;
};
struct BpmPoint
{
	Time TimePoint;

	double BeatLength;
	double Bpm;
};
struct ScrollVelocityMultiplier
{
	Time TimePoint;
	double Multiplier;
};
struct TimeSlice
{
	Time TimePoint;

	int Index;
	
	std::map<Column, std::vector<Note>> Notes;

	std::vector<BpmPoint> BpmPoints;
	std::vector<ScrollVelocityMultiplier> SvMultipliers;
};

struct Chart
{
public: //meta

	Chart();

	std::string Artist;
	std::string SongTitle;

	std::string Charter;
	std::string DifficultyName;

	std::string AudioPath;
	std::string BackgroundPath;

	int KeyAmount;

public: //accessors

	bool PlaceNote(const Time InTime, const Column InColumn, const int InBeatSnap = -1);
	bool PlaceHold(const Time InTimeBegin, const Time InTimeEnd, const Column InColumn, const int InBeatSnapBegin = -1, const int InBeatSnapEnd = -1);

	bool RemoveNote(const Time InTime, const Column InColumn, bool InIgnoreHoldChecks = false);

	Note& InjectNote(const Time InTime, const Column InColumn, const Note::EType InNoteType, const Time InTimeBegin = -1, const Time InTimeEnd = -1, const int InBeatSnap = -1);
	void InjectHold(const Time InTimeBegin, const Time InTimeEnd, const Column InColumn,  const int InBeatSnapBegin = -1, const int InBeatSnapEnd = -1);
	void InjectBpmPoint(const Time InTime, const double InBpm, const double InBeatLength);

	bool IsAPotentialNoteDuplicate(const Time InTime, const Column InColumn);
	TimeSlice& FindOrAddTimeSlice(const Time InTime);
	
	void RegisterTimeSliceHistory(const Time InTime);
	void RegisterTimeSliceHistoryRanged(const Time InTimeBegin, const Time InTimeEnd);

	bool Undo();

	void IterateTimeSlicesInTimeRange(const Time InTimeBegin, const Time InTimeEnd, std::function<void(TimeSlice&)> InWork);
	void IterateNotesInTimeRange(const Time InTimeBegin, const Time InTimeEnd, std::function<void(Note&, const Column)> InWork);
	const std::vector<BpmPoint>& GetBpmPointsRelatedToTimeRange(const Time InTimeBegin, const Time InTimeEnd);

	void DebugPrint();
	void RegisterOnModifiedCallback(std::function<void()> InCallback);

public: //data ownership

	std::map<int, TimeSlice> TimeSlices;
	std::map<int, TimeSlice*> TimeSlicesWithBpmPoints;

	std::stack<std::vector<TimeSlice>> TimeSliceHistory;

	std::vector<BpmPoint> CachedBpmPoints;

private:

	std::function<void()> _OnModified;	
};