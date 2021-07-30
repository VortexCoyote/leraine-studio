#pragma once

#include <map>
#include <vector>
#include <stack>
#include <map>
#include <string>
#include <utility>
#include <functional>
#include <unordered_set>

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

	bool operator==(const BpmPoint& InOther)
	{
		return (TimePoint == InOther.TimePoint);
	}
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

struct NoteReferenceCollection
{
	void PushNote(Column InColumn, Note* InNote);
	void Clear();

	void TrySetMinMaxTime(Time InTime);

	std::unordered_map<Column, std::unordered_set<Note *>> Notes;

	Time MinTimePoint;
	Time MaxTimePoint;

	bool HasNotes = false;

	int NoteAmount = 0;
};

struct Chart
{
public: //meta

	Chart();

	std::string ArtistUnicode;
	std::string Artist;
	
	std::string SongtitleUnicode;
	std::string SongTitle;

	std::string Charter;
	std::string DifficultyName;

	std::string Source;
	std::string Tags;

	std::string BeatmapID = "0";
	std::string BeatmapSetID = "0";

	std::string AudioPath;
	std::string BackgroundPath;

	int KeyAmount = 0;

	float HP = 0;
	float OD = 0;

	std::vector<std::string> InheritedTimingPoints;

public: //accessors

	bool PlaceNote(const Time InTime, const Column InColumn, const int InBeatSnap = -1);
	bool PlaceHold(const Time InTimeBegin, const Time InTimeEnd, const Column InColumn, const int InBeatSnapBegin = -1, const int InBeatSnapEnd = -1);
	bool PlaceBpmPoint(const Time InTime, const double InBpm, const double InBeatLength);

	void BulkPlaceNotes(const std::vector<std::pair<Column, Note>>& InNotes, const bool InSkipHistoryRegistering = false, const bool InSkipOnModified = false);
	void MirrorNotes(NoteReferenceCollection& OutNotes);
	void MirrorNotes(std::vector<std::pair<Column, Note>>& OutNotes);

	bool RemoveNote(const Time InTime, const Column InColumn, const bool InIgnoreHoldChecks = false, const bool InSkipHistoryRegistering = false, const bool InSkipOnModified = false);
	bool RemoveBpmPoint(BpmPoint& InBpmPoint, const bool InSkipHistoryRegistering = false);
	bool BulkRemoveNotes(NoteReferenceCollection& InNotes, const bool InSkipHistoryRegistering = false);

	Note& InjectNote(const Time InTime, const Column InColumn, const Note::EType InNoteType, const Time InTimeBegin = -1, const Time InTimeEnd = -1, const int InBeatSnap = -1, const bool InSkipOnModified = false);
	Note& InjectHold(const Time InTimeBegin, const Time InTimeEnd, const Column InColumn,  const int InBeatSnapBegin = -1, const int InBeatSnapEnd = -1, const bool InSkipOnModified = false);
	BpmPoint* InjectBpmPoint(const Time InTime, const double InBpm, const double InBeatLength);

	Note* MoveNote(const Time InTimeFrom, const Time InTimeTo, const Column InColumnFrom, const Column InColumnTo, const int InNewBeatSnap);
	Note* FindNote(const Time InTime, const Column InColumn);
	bool IsAPotentialNoteDuplicate(const Time InTime, const Column InColumn);
	TimeSlice& FindOrAddTimeSlice(const Time InTime);
	
	void FillNoteCollectionWithAllNotes(NoteReferenceCollection& OutNotes);

	void RevaluateBpmPoint(BpmPoint& InFormerBpmPoint, BpmPoint& InMovedBpmPoint);
	void PushTimeSliceHistoryIfNotAdded(const Time InTime);
	void RegisterTimeSliceHistory(const Time InTime);
	void RegisterTimeSliceHistoryRanged(const Time InTimeBegin, const Time InTimeEnd);

	bool Undo();

	void IterateTimeSlicesInTimeRange(const Time InTimeBegin, const Time InTimeEnd, std::function<void(TimeSlice&)> InWork);
	void IterateNotesInTimeRange(const Time InTimeBegin, const Time InTimeEnd, std::function<void(Note&, const Column)> InWork);

	void IterateAllNotes(std::function<void(Note&, const Column)> InWork);
	void IterateAllBpmPoints(std::function<void(BpmPoint&)> InWork);

	std::vector<BpmPoint*>& GetBpmPointsRelatedToTimeRange(const Time InTimeBegin, const Time InTimeEnd);
	BpmPoint* GetPreviousBpmPointFromTimePoint(const Time InTime);
	BpmPoint* GetNextBpmPointFromTimePoint(const Time InTime);

	void DebugPrint();
	void RegisterOnModifiedCallback(std::function<void(TimeSlice&)> InCallback);

public: //data ownership

	std::map<int, TimeSlice> TimeSlices;

	std::stack<std::vector<TimeSlice>> TimeSliceHistory;

	std::vector<BpmPoint*> CachedBpmPoints;

private:

	std::function<void(TimeSlice&)> _OnModified;	

	int _BpmPointCounter = 0;
	bool _HasNegativePlacedBpmPoint = false;
};