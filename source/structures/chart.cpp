#include "chart.h"

#include <iostream>
#include <algorithm>
#include <set>
#include <unordered_set>
#include <limits>

void NoteReferenceCollection::PushNote(Column InColumn, Note* InNote) 
{
	HasNotes = true;
	NoteAmount++;

	if(ColumnNoteCount.find(InColumn) == ColumnNoteCount.end())
		ColumnNoteCount[InColumn] = 0;

	ColumnNoteCount[InColumn] += 1;
	Notes[InColumn].insert(InNote);

	HighestColumnAmount = std::max(HighestColumnAmount, ColumnNoteCount[InColumn]);

	switch (InNote->Type)
	{
	case Note::EType::Common:
		TrySetMinMaxTime(InNote->TimePoint);
		break;

	case Note::EType::HoldBegin:
		TrySetMinMaxTime(InNote->TimePointEnd);
		break;
	}
}

void NoteReferenceCollection::Clear() 
{
	HasNotes = false;
	NoteAmount = 0;
	HighestColumnAmount = 0;

	Notes.clear();
	ColumnNoteCount.clear();

	MinTimePoint = std::numeric_limits<int>::max();
	MaxTimePoint = std::numeric_limits<int>::min();
}

void NoteReferenceCollection::TrySetMinMaxTime(Time InTime) 
{
	MinTimePoint = std::min(MinTimePoint, InTime);
	MaxTimePoint = std::max(MaxTimePoint, InTime);
}

bool Chart::PlaceNote(const Time InTime, const Column InColumn, const int InBeatSnap)
{
	if (IsAPotentialNoteDuplicate(InTime, InColumn))
		return false;

	RegisterTimeSliceHistory(InTime);
	InjectNote(InTime, InColumn, Note::EType::Common, -1, -1, InBeatSnap);

	return true;
}

bool Chart::PlaceHold(const Time InTimeBegin, const Time InTimeEnd, const Column InColumn, const int InBeatSnap, const int InBeatSnapEnd)
{
	if (InTimeBegin == InTimeEnd)
		return PlaceNote(InTimeBegin, InColumn, InBeatSnap);

	if (IsAPotentialNoteDuplicate(InTimeBegin, InColumn) || IsAPotentialNoteDuplicate(InTimeEnd, InColumn))
		return false;

	RegisterTimeSliceHistoryRanged(InTimeBegin, InTimeEnd);

	InjectHold(InTimeBegin, InTimeEnd, InColumn, InBeatSnap);

	return true;
}

bool Chart::PlaceBpmPoint(const Time InTime, const double InBpm, const double InBeatLength)
{
	RegisterTimeSliceHistory(InTime);
	InjectBpmPoint(InTime, InBpm, InBeatLength);

	return true;
}

void Chart::BulkPlaceNotes(const std::vector<std::pair<Column, Note>> &InNotes, const bool InSkipHistoryRegistering, const bool InSkipOnModified)
{
	Time timePointMin = InNotes.front().second.TimePoint;
	Time timePointMax = InNotes.back().second.TimePoint;

	// - TIMESLICE_LENGTH and + TIMESLICE_LENGTH accounts for potential resnaps (AAAAAA)
	if(!InSkipHistoryRegistering)
		RegisterTimeSliceHistoryRanged(timePointMin - TIMESLICE_LENGTH, timePointMax + TIMESLICE_LENGTH);

	for (const auto &[column, note] : InNotes)
	{
		switch (note.Type)
		{
		case Note::EType::Common:
			InjectNote(note.TimePoint, column, note.Type, -1, -1, -1, InSkipOnModified);
			break;

		case Note::EType::HoldBegin:
			InjectHold(note.TimePointBegin, note.TimePointEnd, column, -1, -1, InSkipOnModified);
			break;
		}
	}
}

void Chart::MirrorNotes(NoteReferenceCollection& OutNotes)
{
	std::vector<std::pair<Column, Note>> bulkOfNotes;

	RegisterTimeSliceHistoryRanged(OutNotes.MinTimePoint, OutNotes.MaxTimePoint);

	for (auto& [column, notes] : OutNotes.Notes)
	{
		Column newColumn = (KeyAmount - 1) - column;

		//this is neccesary since removing an element form a vector will change the contents of a pointer pointing to that element
		std::vector<Note> copiedNotes;
		for (auto &note : notes)
			copiedNotes.push_back(*note);

		for (auto &note : copiedNotes)
		{
			bulkOfNotes.push_back({newColumn, note});
			RemoveNote(note.TimePoint, column, false, true, true);
		}
	}

	BulkPlaceNotes(bulkOfNotes, true, true);

	IterateTimeSlicesInTimeRange(OutNotes.MinTimePoint, OutNotes.MaxTimePoint, [this](TimeSlice& InTimeSlice)
	{
		_OnModified(InTimeSlice);
	});

	OutNotes.Clear();
}

void Chart::MirrorNotes(std::vector<std::pair<Column, Note>>& OutNotes) 
{
	for(auto& [column, notes] : OutNotes)
		column = (KeyAmount - 1) - column;
}

bool Chart::RemoveNote(const Time InTime, const Column InColumn, const bool InIgnoreHoldChecks, const bool InSkipHistoryRegistering, const bool InSkipOnModified)
{
	auto &timeSlice = FindOrAddTimeSlice(InTime);
	auto &noteCollection = timeSlice.Notes[InColumn];

	auto noteIt = std::find_if(noteCollection.begin(), noteCollection.end(), [InTime](const Note &InNote)
							   { return InNote.TimePoint == InTime; });

	if (noteIt == noteCollection.end())
		return false;

	//hold checks
	if (InIgnoreHoldChecks == false && (noteIt->Type == Note::EType::HoldBegin || noteIt->Type == Note::EType::HoldEnd))
	{
		Time holdTimeBegin = noteIt->TimePointBegin;
		Time holdTimedEnd = noteIt->TimePointEnd;

		if (!InSkipHistoryRegistering)
			RegisterTimeSliceHistoryRanged(holdTimeBegin, holdTimedEnd);

		//removes all intermidiate notes
		for (Time time = FindOrAddTimeSlice(holdTimeBegin).TimePoint + TIMESLICE_LENGTH;
			 time <= FindOrAddTimeSlice(holdTimedEnd).TimePoint - TIMESLICE_LENGTH;
			 time += TIMESLICE_LENGTH)
		{
			RemoveNote(time, InColumn, true);
		}

		RemoveNote(holdTimeBegin, InColumn, true);
		RemoveNote(holdTimedEnd, InColumn, true);
	}
	else
	{
		if (!InIgnoreHoldChecks && !InSkipHistoryRegistering)
			RegisterTimeSliceHistory(InTime);

		noteCollection.erase(noteIt);
	}

	if(!InSkipOnModified)
		_OnModified(timeSlice);

	return true;
}

bool Chart::RemoveBpmPoint(BpmPoint &InBpmPoint, const bool InSkipHistoryRegistering)
{
	auto &timeSlice = FindOrAddTimeSlice(InBpmPoint.TimePoint);
	auto &bpmCollection = timeSlice.BpmPoints;

	if (!InSkipHistoryRegistering)
		RegisterTimeSliceHistory(InBpmPoint.TimePoint);

	bpmCollection.erase(std::remove(bpmCollection.begin(), bpmCollection.end(), InBpmPoint), bpmCollection.end());

	CachedBpmPoints.clear();

	_BpmPointCounter--;

	return true;
}

bool Chart::BulkRemoveNotes(NoteReferenceCollection& InNotes, const bool InSkipHistoryRegistering) 
{
	if (!InSkipHistoryRegistering)
		RegisterTimeSliceHistoryRanged(InNotes.MinTimePoint, InNotes.MaxTimePoint);

	for (auto& [column, notes] : InNotes.Notes)
	{
		Column newColumn = (KeyAmount - 1) - column;

		//this is neccesary since removing an element form a vector will change the contents of a pointer pointing to that element
		std::vector<Note> copiedNotes;
		for (auto &note : notes)
			copiedNotes.push_back(*note);

		for (auto &note : copiedNotes)
			RemoveNote(note.TimePoint, column, false, true, true);
	}

	IterateTimeSlicesInTimeRange(InNotes.MinTimePoint, InNotes.MaxTimePoint, [this](TimeSlice& InTimeSlice)
	{
		_OnModified(InTimeSlice);
	});

	InNotes.Clear();

	return true;
}

Note &Chart::InjectNote(const Time InTime, const Column InColumn, const Note::EType InNoteType, const Time InTimeBegin, const Time InTimeEnd, const int InBeatSnap, const bool InSkipOnModified)
{
	auto &timeSlice = FindOrAddTimeSlice(InTime);

	Note note;
	note.Type = InNoteType;
	note.TimePoint = InTime;
	note.BeatSnap = InBeatSnap;

	note.TimePointBegin = InTimeBegin;
	note.TimePointEnd = InTimeEnd;

	timeSlice.Notes[InColumn].push_back(note);
	Note &injectedNoteRef = timeSlice.Notes[InColumn].back();

	std::sort(timeSlice.Notes[InColumn].begin(), timeSlice.Notes[InColumn].end(), [](const auto &lhs, const auto &rhs)
			  { return (lhs.TimePoint < rhs.TimePoint); });

	if(!InSkipOnModified)
		_OnModified(timeSlice);

	return injectedNoteRef;
}

Note &Chart::InjectHold(const Time InTimeBegin, const Time InTimeEnd, const Column InColumn, const int InBeatSnapBegin, const int InBeatSnapEnd, const bool InSkipOnModified)
{
	Note &noteToReturn = InjectNote(InTimeBegin, InColumn, Note::EType::HoldBegin, InTimeBegin, InTimeEnd, InBeatSnapBegin);

	Time startTime = FindOrAddTimeSlice(InTimeBegin).TimePoint + TIMESLICE_LENGTH;
	Time endTime = FindOrAddTimeSlice(InTimeEnd).TimePoint - TIMESLICE_LENGTH;

	for (Time time = startTime; time <= endTime; time += TIMESLICE_LENGTH)
	{
		InjectNote(time, InColumn, Note::EType::HoldIntermediate, InTimeBegin, InTimeEnd);
	}

	if(!InSkipOnModified)
		InjectNote(InTimeEnd, InColumn, Note::EType::HoldEnd, InTimeBegin, InTimeEnd, InBeatSnapEnd);

	return noteToReturn;
}

BpmPoint *Chart::InjectBpmPoint(const Time InTime, const double InBpm, const double InBeatLength)
{
	auto &timeSlice = FindOrAddTimeSlice(InTime);

	BpmPoint bpmPoint;
	bpmPoint.TimePoint = InTime;
	bpmPoint.Bpm = InBpm;
	bpmPoint.BeatLength = InBeatLength;

	timeSlice.BpmPoints.push_back(bpmPoint);

	BpmPoint *bpmPointPtr = &(timeSlice.BpmPoints.back());

	std::sort(timeSlice.BpmPoints.begin(), timeSlice.BpmPoints.end(), [](const auto &lhs, const auto &rhs)
			  { return lhs.TimePoint < rhs.TimePoint; });

	_BpmPointCounter++;

	return bpmPointPtr;
}

Note *Chart::MoveNote(const Time InTimeFrom, const Time InTimeTo, const Column InColumnFrom, const Column InColumnTo, const int InNewBeatSnap)
{
	//have I mentioned that I really dislike handling edge-cases?
	Note noteToRemove = *FindNote(InTimeFrom, InColumnFrom);

	switch (noteToRemove.Type)
	{
	case Note::EType::Common:
	{
		auto &timeSliceFrom = FindOrAddTimeSlice(InTimeFrom);
		auto &timeSliceTo = FindOrAddTimeSlice(InTimeTo);

		if (timeSliceFrom.Index == timeSliceTo.Index)
			RegisterTimeSliceHistory(InTimeFrom);
		else
			RegisterTimeSliceHistoryRanged(InTimeFrom, InTimeTo);

		RemoveNote(InTimeFrom, InColumnFrom, false, true);
		return &(InjectNote(InTimeTo, InColumnTo, Note::EType::Common, -1, -1, InNewBeatSnap));
	}
	break;

	case Note::EType::HoldBegin:
	{
		if (InTimeTo < noteToRemove.TimePointBegin)
			RegisterTimeSliceHistoryRanged(InTimeTo - TIMESLICE_LENGTH, noteToRemove.TimePointEnd);
		else
			RegisterTimeSliceHistoryRanged(noteToRemove.TimePointBegin - TIMESLICE_LENGTH, noteToRemove.TimePointEnd);

		RemoveNote(InTimeFrom, InColumnFrom, false, true);

		return &(InjectHold(InTimeTo, noteToRemove.TimePointEnd, InColumnTo, InNewBeatSnap));
	}
	break;
	case Note::EType::HoldEnd:
	{
		if (InTimeTo > noteToRemove.TimePointBegin)
			RegisterTimeSliceHistoryRanged(noteToRemove.TimePointBegin, InTimeTo + TIMESLICE_LENGTH);
		else
			RegisterTimeSliceHistoryRanged(noteToRemove.TimePointBegin, noteToRemove.TimePointEnd + TIMESLICE_LENGTH);

		int beatSnap = FindNote(noteToRemove.TimePointBegin, InColumnFrom)->BeatSnap;

		RemoveNote(InTimeFrom, InColumnFrom, false, true);

		return &(InjectHold(noteToRemove.TimePointBegin, InTimeTo, InColumnTo, beatSnap));
	}
	break;

	default:
		return nullptr;
		break;
	}
}

Note *Chart::FindNote(const Time InTime, const Column InColumn)
{
	auto &timeSlice = FindOrAddTimeSlice(InTime);
	auto &noteCollection = timeSlice.Notes[InColumn];

	auto noteIt = std::find_if(noteCollection.begin(), noteCollection.end(), [InTime](const Note &InNote)
							   { return InNote.TimePoint == InTime; });

	if (noteIt == noteCollection.end())
		return nullptr;

	return &(*noteIt);
}

void Chart::DebugPrint()
{
	std::cout << DifficultyName << std::endl;
	std::cout << "***************************" << std::endl;

	for (auto [timePoint, slice] : TimeSlices)
	{
		for (auto [column, notes] : slice.Notes)
		{
			for (auto note : notes)
			{
				std::string type = "";
				switch (note.Type)
				{
				case Note::EType::Common:
					type = "common";
					break;
				case Note::EType::HoldBegin:
					type = "hold begin";
					break;
				case Note::EType::HoldIntermediate:
					type = "hold intermediate";
					break;
				case Note::EType::HoldEnd:
					type = "hold end";
					break;

				default:
					break;
				}

				std::cout << timePoint << ":" << std::to_string(note.TimePoint) << " - " << std::to_string(column) << " - " << type << std::endl;
			}
		}
	}

	std::cout << std::endl;
}

void Chart::RegisterOnModifiedCallback(std::function<void(TimeSlice &)> InCallback)
{
	_OnModified = InCallback;
}

bool Chart::IsAPotentialNoteDuplicate(const Time InTime, const Column InColumn)
{
	auto &notes = FindOrAddTimeSlice(InTime).Notes[InColumn];
	return (std::find_if(notes.begin(), notes.end(), [InTime](const Note &InNote)
						 { return InNote.TimePoint == InTime; }) != notes.end());
}

TimeSlice &Chart::FindOrAddTimeSlice(const Time InTime)
{
	int index = InTime / TIMESLICE_LENGTH;
	if (TimeSlices.find(index) == TimeSlices.end())
	{
		TimeSlices[index].TimePoint = index * TIMESLICE_LENGTH;
		TimeSlices[index].Index = index;
	}

	return TimeSlices[index];
}

void Chart::FillNoteCollectionWithAllNotes(NoteReferenceCollection& OutNotes) 
{
	OutNotes.Clear();

	IterateAllNotes([&OutNotes](Note& InNote, Column InColumn)
	{
		OutNotes.PushNote(InColumn, &InNote);
	});
}

void Chart::PushTimeSliceHistoryIfNotAdded(const Time InTime)
{
	if (TimeSliceHistory.size() == 0)
		return;

	auto &collection = TimeSliceHistory.top();

	for (auto &timeSlice : collection)
	{
		if (InTime >= timeSlice.TimePoint && InTime <= timeSlice.TimePoint + TIMESLICE_LENGTH)
			return;
	}

	collection.push_back(FindOrAddTimeSlice(InTime));
}

void Chart::RevaluateBpmPoint(BpmPoint &InFormerBpmPoint, BpmPoint &InMovedBpmPoint)
{
	auto &formerTimeSlice = FindOrAddTimeSlice(InFormerBpmPoint.TimePoint);
	auto &newTimeSlice = FindOrAddTimeSlice(InMovedBpmPoint.TimePoint);

	auto &formerBpmCollection = formerTimeSlice.BpmPoints;

	if (formerTimeSlice.Index != newTimeSlice.Index)
	{
		BpmPoint bpmPointToAdd = InMovedBpmPoint;

		formerBpmCollection.erase(std::remove(formerBpmCollection.begin(), formerBpmCollection.end(), InMovedBpmPoint), formerBpmCollection.end());
		CachedBpmPoints.clear();

		TimeSliceHistory.top().push_back(newTimeSlice);

		InjectBpmPoint(bpmPointToAdd.TimePoint, bpmPointToAdd.Bpm, bpmPointToAdd.BeatLength);

		_BpmPointCounter--;
	}
}

void Chart::RegisterTimeSliceHistory(const Time InTime)
{
	TimeSliceHistory.push({FindOrAddTimeSlice(InTime)});
}

void Chart::RegisterTimeSliceHistoryRanged(const Time InTimeBegin, const Time InTimeEnd)
{
	std::vector<TimeSlice> timeSlices;

	IterateTimeSlicesInTimeRange(InTimeBegin, InTimeEnd, [&timeSlices](TimeSlice &InTimeSlice)
								 { timeSlices.push_back(InTimeSlice); });

	TimeSliceHistory.push(timeSlices);
}

bool Chart::Undo()
{
	if (TimeSliceHistory.empty())
		return false;

	for (auto &timeSlice : TimeSliceHistory.top())
		_OnModified(TimeSlices[timeSlice.Index] = timeSlice);

	TimeSliceHistory.pop();

	return true;
}

void Chart::IterateTimeSlicesInTimeRange(const Time InTimeBegin, const Time InTimeEnd, std::function<void(TimeSlice &)> InWork)
{
	if (InTimeBegin > InTimeEnd)
	{
		for (TimeSlice *timeSlice = &FindOrAddTimeSlice(InTimeBegin);
			 timeSlice->TimePoint > InTimeEnd - TIMESLICE_LENGTH;
			 timeSlice = &FindOrAddTimeSlice(timeSlice->TimePoint - TIMESLICE_LENGTH))
		{
			InWork(*timeSlice);
		}

		return;
	}

	for (TimeSlice *timeSlice = &FindOrAddTimeSlice(InTimeBegin);
		 timeSlice->TimePoint <= InTimeEnd;
		 timeSlice = &FindOrAddTimeSlice(timeSlice->TimePoint + TIMESLICE_LENGTH))
	{
		InWork(*timeSlice);
	}
}

void Chart::IterateNotesInTimeRange(const Time InTimeBegin, const Time InTimeEnd, std::function<void(Note &, const Column)> InWork)
{
	for (TimeSlice *timeSlice = &FindOrAddTimeSlice(InTimeBegin);
		 timeSlice->TimePoint <= InTimeEnd;
		 timeSlice = &FindOrAddTimeSlice(timeSlice->TimePoint + TIMESLICE_LENGTH))
	{
		for (auto &[column, notes] : timeSlice->Notes)
		{
			for (auto &note : notes)
			{
				if (note.TimePoint >= InTimeBegin && note.TimePoint <= InTimeEnd)
					InWork(note, column);
			}
		}
	}
}

void Chart::IterateAllNotes(std::function<void(Note &, const Column)> InWork)
{
	for (auto &[ID, timeSlice] : TimeSlices)
	{
		for (auto &[column, notes] : timeSlice.Notes)
		{
			for (auto &note : notes)
			{
				InWork(note, column);
			}
		}
	}
}

void Chart::IterateAllBpmPoints(std::function<void(BpmPoint &)> InWork)
{
	for (auto &[ID, timeSlice] : TimeSlices)
	{
		for (auto &bpmPoint : timeSlice.BpmPoints)
		{
			InWork(bpmPoint);
		}
	}
}

std::vector<BpmPoint *> &Chart::GetBpmPointsRelatedToTimeRange(const Time InTimeBegin, const Time InTimeEnd)
{
	CachedBpmPoints.clear();

	if (!_BpmPointCounter)
		return CachedBpmPoints;

	Time newTimeBegin = InTimeBegin - TIMESLICE_LENGTH;

	for (Time time = newTimeBegin; time >= -10000; time -= TIMESLICE_LENGTH)
	{
		auto &timeSlice = FindOrAddTimeSlice(time);
		if (!timeSlice.BpmPoints.empty())
		{
			newTimeBegin = time;
			break;
		}
	}

	IterateTimeSlicesInTimeRange(newTimeBegin, InTimeEnd, [this](TimeSlice &InTimeSlice)
								 {
									 if (InTimeSlice.BpmPoints.empty())
										 return;

									 for (auto &bpmPoint : InTimeSlice.BpmPoints)
										 CachedBpmPoints.push_back(&bpmPoint);
								 });

	if (CachedBpmPoints.empty())
		return GetBpmPointsRelatedToTimeRange(InTimeBegin, InTimeEnd + TIMESLICE_LENGTH);

	return CachedBpmPoints;
}

BpmPoint *Chart::GetPreviousBpmPointFromTimePoint(const Time InTime)
{
	BpmPoint *foundBpmPoint = nullptr;
	BpmPoint *previousBpmPoint = nullptr;

	IterateTimeSlicesInTimeRange(0, InTime, [this, InTime, &foundBpmPoint, &previousBpmPoint](TimeSlice &InTimeSlice)
								 {
									 if (InTimeSlice.BpmPoints.empty())
										 return;

									 for (auto &bpmPoint : InTimeSlice.BpmPoints)
										 if (bpmPoint.TimePoint < InTime)
											 previousBpmPoint = &bpmPoint;
								 });

	foundBpmPoint = previousBpmPoint;

	return foundBpmPoint;
}

BpmPoint *Chart::GetNextBpmPointFromTimePoint(const Time InTime)
{
	if (!_BpmPointCounter)
		return nullptr;

	auto &relevantTimeSlice = FindOrAddTimeSlice(InTime);

	auto timeSliceIt = TimeSlices.find(relevantTimeSlice.Index);

	while (timeSliceIt != TimeSlices.end())
	{
		auto &currentTimeSlice = *timeSliceIt;

		for (auto &bpmPoint : currentTimeSlice.second.BpmPoints)
			if (bpmPoint.TimePoint > InTime)
				return &bpmPoint;

		timeSliceIt++;
	}

	return nullptr;
}

Chart::Chart()
{
	_OnModified = [](TimeSlice &InOutTimeSlice) {};
}

