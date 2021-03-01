#include "beat-module.h"

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

void BeatModule::AssignNotesToSnapsInChart(Chart* const InChart)
{
	Time timeBegin = 0;
	Time timeEnd = InChart->TimeSlices.rbegin()->second.TimePoint + TIMESLICE_LENGTH;

	for (Time time = timeBegin; time <= timeEnd; time += TIMESLICE_LENGTH)
	{
		auto& slice = InChart->FindOrAddTimeSlice(time);

		AssignNotesToSnapsInTimeSlice(InChart, slice);
	}
}

void BeatModule::AssignNotesToSnapsInTimeSlice(Chart* const InChart, TimeSlice& InOutTimeSlice) 
{
	GenerateTimeRangeBeatLines(InOutTimeSlice.TimePoint, InOutTimeSlice.TimePoint + TIMESLICE_LENGTH, InChart, 48);
	GenerateTimeRangeBeatLines(InOutTimeSlice.TimePoint, InOutTimeSlice.TimePoint + TIMESLICE_LENGTH, InChart, 5, true);

	std::sort(_OnFieldBeatLines.begin(), _OnFieldBeatLines.end(), [](const auto& lhs, const auto& rhs) { return lhs.TimePoint < rhs.TimePoint; });

	for (auto& column : InOutTimeSlice.Notes)
	{
		for (auto& note : column.second)
		{
			auto attachedBeatLine = GetClosestBeatLineToTimePoint(note.TimePoint);
			note.BeatSnap = GetBeatSnap(attachedBeatLine, attachedBeatLine.BeatDivision);
		}
	}

	_OnFieldBeatLines.clear();
}

void BeatModule::GenerateTimeRangeBeatLines(const Time InTimeBegin, const Time InTimeEnd, Chart* const InChart, const int InBeatDivision, const bool InSkipClearCollection)
{
	//edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases edge cases
	if(!InSkipClearCollection)
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

		const double timeBetweenSnaps = bpmPoint.BeatLength / double(InBeatDivision);
		double firstPosition = double(bpmPoint.TimePoint) + ceil(double(timeBegin - bpmPoint.TimePoint) / timeBetweenSnaps) * timeBetweenSnaps;

		if (timeBegin == InTimeBegin)
			firstPosition -= timeBetweenSnaps;

		if (timeEnd == InTimeEnd)
			timeEnd += Time(timeBetweenSnaps);

		int beatCount = (int)std::max(0.0, double(firstPosition - double(bpmPoint.TimePoint)) / timeBetweenSnaps + 0.5);
		
		for (double timePosition = firstPosition; timePosition + 0.5 < double(timeEnd); timePosition += timeBetweenSnaps)
		{
			Time actualTime = Time(timePosition);
			Time analyticalTime = bpmPoint.TimePoint + Time(bpmPoint.BeatLength * double(beatCount)) / InBeatDivision; //this is apparently more accurate than the one above, so it's used for the actual beatlines

			if (actualTime < bpmPoint.TimePoint)
				_OnFieldBeatLines.push_back({ actualTime, -1, InBeatDivision, -1});
			else
			{
				_OnFieldBeatLines.push_back({ analyticalTime, beatCount, InBeatDivision, GetBeatSnap(beatCount, InBeatDivision) });
				beatCount++;
			}
		}
		index++;
	}
}

void BeatModule::GenerateBeatLinesFromTimePointIfInvalid(Chart* const InChart, const Time InTime) 
{
	const Time timeBegin = _OnFieldBeatLines.front().TimePoint;
	const Time timeEnd = _OnFieldBeatLines.back().TimePoint;

	if(InTime < timeBegin)
	{
		Time deltaTime = abs(timeBegin - InTime) + TIMESLICE_LENGTH;

		GenerateTimeRangeBeatLines(timeBegin - deltaTime, timeBegin, InChart, 48, true);
		GenerateTimeRangeBeatLines(timeBegin - deltaTime, timeBegin, InChart, 5, true);
	}

	if(InTime > timeEnd)
	{
		Time deltaTime = abs(timeEnd - InTime) + TIMESLICE_LENGTH;

		GenerateTimeRangeBeatLines(timeEnd, timeEnd + deltaTime, InChart, 48, true);
		GenerateTimeRangeBeatLines(timeEnd, timeEnd + deltaTime, InChart, 5, true);
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
	return GlobalFunctions::FloatCompare(fmodf(float(InBeatCount), occurrences) + occurrences, occurrences, 0.001f);
}

BeatLine BeatModule::GetClosestBeatLineToTimePoint(const Time InTimePoint) 
{
	BeatLine attachedBeatLine = _OnFieldBeatLines.back();

	for (auto beatLine = _OnFieldBeatLines.rbegin(); beatLine != _OnFieldBeatLines.rend(); ++beatLine)
	{
		if (beatLine->TimePoint + 2 < InTimePoint)
				break;

		attachedBeatLine = *beatLine;
	}

	return attachedBeatLine;
}
