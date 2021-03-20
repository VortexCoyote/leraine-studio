#include "beat-module.h"

#include <cmath>

bool BeatModule::StartUp()
{
	_OnFieldBeatLines.reserve(10000);

	_LegalSnaps.insert(1);
	_LegalSnaps.insert(2);
	_LegalSnaps.insert(3);
	_LegalSnaps.insert(4);
	_LegalSnaps.insert(5);
	_LegalSnaps.insert(6);
	_LegalSnaps.insert(8);
	_LegalSnaps.insert(12);
	_LegalSnaps.insert(16);
	_LegalSnaps.insert(24);
	_LegalSnaps.insert(48);
	
	return true;
}

void BeatModule::AssignSnapsToNotesInChart(Chart* const InChart)
{
	Time timeBegin = 0;
	Time timeEnd = InChart->TimeSlices.rbegin()->second.TimePoint + TIMESLICE_LENGTH;

	for (Time time = timeBegin; time <= timeEnd; time += TIMESLICE_LENGTH)
	{
		auto& slice = InChart->FindOrAddTimeSlice(time);

		GenerateTimeRangeBeatLines(slice.TimePoint, slice.TimePoint + TIMESLICE_LENGTH, InChart, 48);

		for (auto& column : slice.Notes)
		{
			for (auto& note : column.second)
			{
				if (note.Type == Note::EType::HoldEnd || note.Type == Note::EType::HoldIntermediate)
					continue;

				BeatLine attachedBeatLine = _OnFieldBeatLines.back();

				for (auto beatLine = _OnFieldBeatLines.rbegin(); beatLine != _OnFieldBeatLines.rend(); ++beatLine)
				{
					if (beatLine->TimePoint + 1 < note.TimePoint)
						break;

					attachedBeatLine = *beatLine;
				}

				note.BeatSnap = GetBeatSnap(attachedBeatLine, attachedBeatLine.BeatDivision);
			}
		}
	}

	_OnFieldBeatLines.clear();
}

void BeatModule::GenerateTimeRangeBeatLines(const Time InTimeBegin, const Time InTimeEnd, Chart* const InChart, const int InBeatDivision)
{
	//edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases
	
	_OnFieldBeatLines.clear();
	
	const auto& bpmPoints = InChart->GetBpmPointsRelatedToTimeRange(InTimeBegin, InTimeEnd);

	size_t index = 0;
	for (const auto& bpmPoint : bpmPoints)
	{
		Time timeBegin = InTimeBegin;
		Time timeEnd = InTimeEnd;

		if (index != 0)
			timeBegin = bpmPoint.TimePoint;

		if (index + 1 <= bpmPoints.size() - 1)
			timeEnd = bpmPoints[index + 1].TimePoint;

		const float timeBetweenSnaps = 60000.f / bpmPoint.Bpm / float(InBeatDivision);
		float firstPosition = bpmPoint.TimePoint + ceilf((timeBegin - bpmPoint.TimePoint) / timeBetweenSnaps) * timeBetweenSnaps + 0.5f;

		if (timeBegin == InTimeBegin)
			firstPosition -= timeBetweenSnaps;

		if (timeEnd == InTimeEnd)
			timeEnd += timeBetweenSnaps;

		int beatCount = std::max(0.f, float(firstPosition - float(bpmPoint.TimePoint)) / timeBetweenSnaps + 0.5f);

		for (float timePosition = firstPosition; timePosition + 0.5f < timeEnd; timePosition += timeBetweenSnaps)
		{
			Time actualTime = Time(timePosition + 0.5f) - 1;

			if (actualTime + 0.5f < bpmPoint.TimePoint)
				_OnFieldBeatLines.push_back({ actualTime , -1, InBeatDivision, -1});
			else
			{
				_OnFieldBeatLines.push_back({ actualTime, beatCount, InBeatDivision, GetBeatSnap(beatCount, InBeatDivision) });
				beatCount++;
			}
		}
		index++;
	}
}

void BeatModule::IterateThroughBeatlines(std::function<void(const BeatLine&)> InWork)
{
	for (BeatLine beatLine : _OnFieldBeatLines)
		InWork(beatLine);
}

int BeatModule::GetBeatSnap(const BeatLine& InBeatLine, const int InBeatDivision)
{
	return GetBeatSnap(InBeatLine.BeatCount, InBeatDivision);
}

int BeatModule::GetBeatSnap(const int InBeatCount, const int InBeatDivision)
{
	const int relativeBeatCount = InBeatCount % InBeatDivision;

	//1/1 - 1/16
	if (IsBeatThisDivision(relativeBeatCount, InBeatDivision, 1))
		return 1;

	if (IsBeatThisDivision(relativeBeatCount, InBeatDivision, 2))
		return 2;

	if (IsBeatThisDivision(relativeBeatCount, InBeatDivision, 3))
		return 3;

	if (IsBeatThisDivision(relativeBeatCount, InBeatDivision, 4))
		return 4;

	if (IsBeatThisDivision(relativeBeatCount, InBeatDivision, 6))
		return 6;

	if (IsBeatThisDivision(relativeBeatCount, InBeatDivision, 8))
		return 8;

	if (IsBeatThisDivision(relativeBeatCount, InBeatDivision, 12))
		return 12;

	if (IsBeatThisDivision(relativeBeatCount, InBeatDivision, 16))
		return 16;

	//everything else
	return  -1;
}

BeatLine BeatModule::GetNextBeatLine(const Time InTime)
{
	return GetCurrentBeatLine(InTime, 1, false);

}

BeatLine BeatModule::GetPreviousBeatLine(const Time InTime)
{
	return GetCurrentBeatLine(InTime, -1, true);
}

BeatLine BeatModule::GetCurrentBeatLine(const Time InTime, const Time InBias, const bool InScanDown)
{
	Time time = InTime + InBias;
	BeatLine selectedBeatline;

	if (InScanDown)
	{
		for (auto beatLine = _OnFieldBeatLines.rbegin(); beatLine != _OnFieldBeatLines.rend(); ++beatLine)
		{
			selectedBeatline = *beatLine;
			if (time > beatLine->TimePoint)
				break;
		}
	}
	else
	{
		for (const auto& beatLine : _OnFieldBeatLines)
		{
			selectedBeatline = beatLine;
			if (time < beatLine.TimePoint)
				break;
		}
	}

	return selectedBeatline;
}

const int BeatModule::GetNextSnap(const int InCurrentSnap)
{
	auto it = _LegalSnaps.find(InCurrentSnap);

	it++;

	if (it == _LegalSnaps.end())
		return InCurrentSnap;

	return *it;
}

const int BeatModule::GetPreviousSnap(const int InCurrentSnap)
{
	auto it = _LegalSnaps.find(InCurrentSnap);

	if (it == _LegalSnaps.begin())
		return InCurrentSnap;

	it--;

	return *it;
}

bool BeatModule::IsBeatThisDivision(const int InBeatCount, const int InBeatDivision, const int InDenominator)
{
	const float occurrences = (float(InBeatDivision) / float(InDenominator));
	return GlobalFunctions::FloatCompare(fmodf(InBeatCount, occurrences) + occurrences, occurrences, 0.001f);
}
