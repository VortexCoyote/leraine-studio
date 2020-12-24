#include "chart.h"

#include <iostream>
#include <algorithm>

bool Chart::PlaceNote(const Time InTime, const Column InColumn, const int InBeatSnap)
{
	if (IsAPotentialNoteDuplicate(InTime, InColumn))
		return false;

	RegisterTimeSliceHistory(InTime);
	InjectNote(InTime, InColumn, Note::EType::Common, -1, -1, InBeatSnap);

	_OnModified();

	return true;
}

bool Chart::PlaceHold(const Time InTimeBegin, const Time InTimeEnd, const Column InColumn, const int InBeatSnap, const int InBeatSnapEnd)
{
	if(InTimeBegin == InTimeEnd)
		return PlaceNote(InTimeBegin, InColumn, InBeatSnap);
	
	if (IsAPotentialNoteDuplicate(InTimeBegin, InColumn) || IsAPotentialNoteDuplicate(InTimeEnd, InColumn))
		return false;

	RegisterTimeSliceHistoryRanged(InTimeBegin, InTimeEnd);

	InjectHold(InTimeBegin, InTimeEnd, InColumn, InBeatSnap);

	_OnModified();

	return true;
}

bool Chart::RemoveNote(const Time InTime, const Column InColumn, bool InIgnoreHoldChecks)
{
	auto& noteCollection = FindOrAddTimeSlice(InTime).Notes[InColumn];

	auto noteIt = std::find_if(noteCollection.begin(), noteCollection.end(), [InTime](const Note& InNote) { return InNote.TimePoint == InTime; });

	if(noteIt == noteCollection.end())
		return false;

	//hold checks
	if( InIgnoreHoldChecks == false && (noteIt->Type == Note::EType::HoldBegin || noteIt->Type == Note::EType::HoldEnd))
	{
		Time holdTimeBegin = noteIt->TimePointBegin;
		Time holdTimedEnd = noteIt->TimePointEnd;

		RegisterTimeSliceHistoryRanged(holdTimeBegin, holdTimedEnd);

		//removes all intermidiate notes
		for(Time time = FindOrAddTimeSlice(holdTimeBegin).TimePoint + TIMESLICE_LENGTH; 
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
		if(!InIgnoreHoldChecks)
			RegisterTimeSliceHistory(InTime);
		
		noteCollection.erase(noteIt);	
	}
	
	_OnModified();

	return true;
}

Note& Chart::InjectNote(const Time InTime, const Column InColumn, const Note::EType InNoteType, const Time InTimeBegin, const Time InTimeEnd, const int InBeatSnap)
{
	auto& timeSlice = FindOrAddTimeSlice(InTime);

	Note note;
	note.Type = InNoteType;
	note.TimePoint = InTime;
	note.BeatSnap = InBeatSnap;

	note.TimePointBegin = InTimeBegin;
	note.TimePointEnd = InTimeEnd;

	timeSlice.Notes[InColumn].push_back(note);

	std::sort(timeSlice.Notes[InColumn].begin(), timeSlice.Notes[InColumn].end(), [](const auto& lhs, const auto& rhs) { return (lhs.TimePoint < rhs.TimePoint); });

	return note;
}

void Chart::InjectHold(const Time InTimeBegin, const Time InTimeEnd, const Column InColumn, const int InBeatSnapBegin, const int InBeatSnapEnd)
{
	InjectNote(InTimeBegin, InColumn, Note::EType::HoldBegin, InTimeBegin, InTimeEnd, InBeatSnapBegin);

	Time startTime = FindOrAddTimeSlice(InTimeBegin).TimePoint + TIMESLICE_LENGTH;
	Time endTime = FindOrAddTimeSlice(InTimeEnd).TimePoint - TIMESLICE_LENGTH;

	for (Time time = startTime; time <= endTime; time += TIMESLICE_LENGTH)
	{
		InjectNote(time, InColumn, Note::EType::HoldIntermediate, InTimeBegin, InTimeEnd);
	}

	InjectNote(InTimeEnd, InColumn, Note::EType::HoldEnd, InTimeBegin, InTimeEnd, InBeatSnapEnd);
}

void Chart::InjectBpmPoint(const Time InTime, const float InBpm)
{
	auto& timeSlice = FindOrAddTimeSlice(InTime);

	BpmPoint bpmPoint;
	bpmPoint.TimePoint = InTime;
	bpmPoint.Bpm = InBpm;

	timeSlice.BpmPoints.push_back(bpmPoint);

	std::sort(timeSlice.BpmPoints.begin(), timeSlice.BpmPoints.end(), [](const auto& lhs, const auto& rhs) { return lhs.TimePoint < rhs.TimePoint; });

	TimeSlicesWithBpmPoints[timeSlice.Index] = &timeSlice;
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

void Chart::RegisterOnModifiedCallback(std::function<void()> InCallback) 
{
	_OnModified = InCallback;
}

bool Chart::IsAPotentialNoteDuplicate(const Time InTime, const Column InColumn)
{
	auto& notes = FindOrAddTimeSlice(InTime).Notes[InColumn];
	return (std::find_if(notes.begin(), notes.end(), [InTime](const Note& InNote)
	{
		return InNote.TimePoint == InTime;
	}) != notes.end());
}

TimeSlice& Chart::FindOrAddTimeSlice(const Time InTime)
{
	int index = InTime / TIMESLICE_LENGTH;
	if (TimeSlices.find(index) == TimeSlices.end())
	{
		TimeSlices[index].TimePoint = index * TIMESLICE_LENGTH;
		TimeSlices[index].Index = index;
	}

	return TimeSlices[index];
}

void Chart::RegisterTimeSliceHistory(const Time InTime) 
{
	TimeSliceHistory.push({FindOrAddTimeSlice(InTime)});
}

void Chart::RegisterTimeSliceHistoryRanged(const Time InTimeBegin, const Time InTimeEnd) 
{
	std::vector<TimeSlice> timeSlices;

	IterateTimeSlicesInTimeRange(InTimeBegin, InTimeEnd, [&timeSlices](TimeSlice& InTimeSlice)
	{
		timeSlices.push_back(InTimeSlice);
	});

	TimeSliceHistory.push(timeSlices);
}

bool Chart::Undo() 
{
	if(TimeSliceHistory.empty())
		return false;

	for(auto& timeSlice : TimeSliceHistory.top())
		TimeSlices[timeSlice.Index] = timeSlice;

	TimeSliceHistory.pop();

	_OnModified();

	return true;
}

void Chart::IterateTimeSlicesInTimeRange(const Time InTimeBegin, const Time InTimeEnd, std::function<void(TimeSlice&)> InWork)
{
	for (TimeSlice* timeSlice = &FindOrAddTimeSlice(InTimeBegin);
		timeSlice->TimePoint <= InTimeEnd;
		timeSlice = &FindOrAddTimeSlice(timeSlice->TimePoint + TIMESLICE_LENGTH))
	{
		InWork(*timeSlice);
	}
}

void Chart::IterateNotesInTimeRange(const Time InTimeBegin, const Time InTimeEnd, std::function<void(Note&, const Column)> InWork) 
{
	for (TimeSlice* timeSlice = &FindOrAddTimeSlice(InTimeBegin);
	timeSlice->TimePoint <= InTimeEnd;
	timeSlice = &FindOrAddTimeSlice(timeSlice->TimePoint + TIMESLICE_LENGTH))
	{
		for (auto& [column, notes] : timeSlice->Notes)
		{
			for (auto& note : notes)
			{
				if(note.TimePoint >= InTimeBegin && note.TimePoint <= InTimeEnd)
					InWork(note, column);
			}
		}
	}
}

const std::vector<BpmPoint>& Chart::GetBpmPointsRelatedToTimeRange(const Time InTimeBegin, const Time InTimeEnd)
{
	//TODO: make this good please (I seriously can't believe this works)
	CachedBpmPoints.clear();

	if (TimeSlicesWithBpmPoints.empty())
		return CachedBpmPoints;

	IterateTimeSlicesInTimeRange(InTimeBegin, InTimeEnd, [this](TimeSlice& InTimeSlice)
	{
		if (InTimeSlice.BpmPoints.empty())
			return;

		for (auto bpmPoint : InTimeSlice.BpmPoints)
			CachedBpmPoints.push_back(bpmPoint);
	});

	//this is actually retarded
	int index = FindOrAddTimeSlice(InTimeBegin).Index;
	
	TimeSlicesWithBpmPoints[index];
	auto it = TimeSlicesWithBpmPoints.find(index);
	if (it != TimeSlicesWithBpmPoints.begin())
	{
		if (--it != TimeSlicesWithBpmPoints.end())
			CachedBpmPoints.push_back((*it).second->BpmPoints.back());
	}

	if (FindOrAddTimeSlice(InTimeBegin).BpmPoints.empty())
		TimeSlicesWithBpmPoints.erase(index);

	if (CachedBpmPoints.empty())
		CachedBpmPoints.push_back(TimeSlicesWithBpmPoints.begin()->second->BpmPoints[0]);

	std::sort(CachedBpmPoints.begin(), CachedBpmPoints.end(), [](const auto& lhs, const auto& rhs) { return lhs.TimePoint < rhs.TimePoint; });

	return CachedBpmPoints;
}

void ChartSet::AddChart(Chart* const InChart)
{
	Charts[InChart->DifficultyName] = InChart;
}

bool ChartSet::IsEmpty()
{
	return Charts.empty();
}

Chart::Chart() 
{
	_OnModified = [](){};
}
