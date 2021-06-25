#include "chart-parser-module.h"

#include <filesystem>
#include <sstream>
#include <algorithm>

#define PARSE_COMMA_VALUE(stringstream, target) stringstream >> target; if (stringstream.peek() == ',') stringstream.ignore()

void ChartParserModule::SetCurrentChartPath(const std::string InPath)
{
	_CurrentChartPath = InPath;
}

Chart* ChartParserModule::ParseAndGenerateChartSet(std::string InPath)
{
	_CurrentChartPath = InPath;
	std::ifstream chartFile(InPath);

	return ParseChartOsuImpl(chartFile, InPath);
}

Chart* ChartParserModule::ParseChartOsuImpl(std::ifstream& InIfstream, std::string InPath)
{
	Chart* chart = new Chart();

	std::string line;

	std::filesystem::path path = InPath;
	std::string parentPath = path.parent_path().string();

	while (std::getline(InIfstream, line))
	{
		if (line == "[General]")
		{
			std::string metadataLine;

			while (std::getline(InIfstream, metadataLine))
			{
				if (metadataLine.empty())
					break;

				std::string meta;
				std::string value;

				int pointer = 0;

				while (metadataLine[pointer] != ':')
					meta += metadataLine[pointer++];

				pointer++;
				pointer++;

				while (metadataLine[pointer] != '\0')
					value += metadataLine[pointer++];

				if (meta == "AudioFilename")
				{
					std::string songPath = parentPath + "\\" + value;
					chart->AudioPath = songPath;
					//chartData->audioFileName = value;
				}
			}
		}

		if (line == "[Metadata]")
		{
			std::string metadataLine;

			while (std::getline(InIfstream, metadataLine))
			{
				if (metadataLine.empty())
					break;

				std::string meta;
				std::string value;

				int pointer = 0;

				while (metadataLine[pointer] != ':')
					meta += metadataLine[pointer++];

				pointer++;

				while (metadataLine[pointer] != '\0')
					value += metadataLine[pointer++];

				if (meta == "Title")
					chart->SongTitle = value;

				if (meta == "Artist")
					chart->Artist = value;

				if (meta == "Version")
					chart->DifficultyName = value;

				if (meta == "Creator")
					chart->Charter = value;
			}
		}

		if (line == "[Difficulty]")
		{
			std::string difficultyLine;

			while (std::getline(InIfstream, difficultyLine))
			{
				std::string type;
				std::string value;

				int pointer = 0;

				if (difficultyLine.empty())
					break;

				while (difficultyLine[pointer] != ':')
					type += difficultyLine[pointer++];

				pointer++;

				while (difficultyLine[pointer] != '\0')
					value += difficultyLine[pointer++];

				if (type == "CircleSize")
					chart->KeyAmount = int(std::stof(value));

				if (difficultyLine.empty())
					break;
			}
		}

		if (line == "[Events]")
		{
			std::string timePointLine;

			while (std::getline(InIfstream, timePointLine))
			{
				if (timePointLine == "")
					continue;

				if (timePointLine == "[TimingPoints]")
				{
					line = "[TimingPoints]";
					break;
				}

				if (timePointLine[0] == '/' && timePointLine[1] == '/')
					continue;

				std::string background;
				int charIndex = 0;

				while (timePointLine[charIndex++] != '"');

				do
					background += timePointLine[charIndex];
				while (timePointLine[++charIndex] != '"');

				chart->BackgroundPath = parentPath + "\\" + background;
				
				break;
			}
		}

		if (line == "[TimingPoints]")
		{
			std::string timePointLine;
			while (InIfstream >> line)
			{
				if (line == "[HitObjects]" || line == "[Colours]")
					break;

				std::stringstream timePointStream(line);

				double timePoint;
				double beatLength;
				int meter, sampleSet, sampleIndex, volume, uninherited, effects;

				PARSE_COMMA_VALUE(timePointStream, timePoint);
				PARSE_COMMA_VALUE(timePointStream, beatLength);
				PARSE_COMMA_VALUE(timePointStream, meter);
				PARSE_COMMA_VALUE(timePointStream, sampleSet);
				PARSE_COMMA_VALUE(timePointStream, sampleIndex);
				PARSE_COMMA_VALUE(timePointStream, volume);
				PARSE_COMMA_VALUE(timePointStream, uninherited);
				PARSE_COMMA_VALUE(timePointStream, effects);

				//BPMData* bpmData = new BPMData();
				//bpmData->BPMSaved = bpm;

				if (beatLength < 0)
					continue;

				double bpm = 60000.0 / beatLength;

				//bpmData->BPM = bpm;
				//bpmData->timePoint = int(timePoint);
				//bpmData->meter = meter;
				//bpmData->uninherited = uninherited;

				chart->InjectBpmPoint(Time(timePoint), bpm, beatLength);
			}
		}

		if (line == "[HitObjects]")
		{
			std::string noteLine;
			while (std::getline(InIfstream, noteLine))
			{
				if (noteLine == "")
					continue;

				std::stringstream noteStream(noteLine);
				int column, y, timePoint, noteType, hitSound, timePointEnd;

				PARSE_COMMA_VALUE(noteStream, column);
				PARSE_COMMA_VALUE(noteStream, y);
				PARSE_COMMA_VALUE(noteStream, timePoint);
				PARSE_COMMA_VALUE(noteStream, noteType);
				PARSE_COMMA_VALUE(noteStream, hitSound);
				PARSE_COMMA_VALUE(noteStream, timePointEnd);

				int parsedColumn = std::clamp(floor(float(column) * (float(chart->KeyAmount) / 512.f)), 0.f, float(chart->KeyAmount) - 1.f);

				if (noteType == 128)
				{
					chart->InjectHold(timePoint, timePointEnd, parsedColumn);
				}
				else if (noteType == 1 || noteType == 5)
				{
					chart->InjectNote(timePoint, parsedColumn, Note::EType::Common);
				}
			}
		}
	}


	return chart;
}

Chart* ChartParserModule::ParseChartStepmaniaImpl(std::ifstream& InIfstream, std::string InPath)
{
	return nullptr;
}

void ChartParserModule::ExportChartSet(Chart* InChart, const Time InSongLength)
{
	std::ofstream chartFile(_CurrentChartPath);

	ExportChartOsuImpl(InChart, InSongLength, chartFile);
}

void ChartParserModule::ExportChartOsuImpl(Chart* InChart, const Time InSongLength, std::ofstream& InOfStream)
{	
	std::string backgroundFileName;
	for (std::string::reverse_iterator rit = InChart->BackgroundPath.rbegin(); rit != InChart->BackgroundPath.rend() && *rit != '/' && *rit != '\\'; ++rit)
		backgroundFileName.insert(backgroundFileName.begin(), *rit);

	std::string audioFileName;
	for (std::string::reverse_iterator rit = InChart->AudioPath.rbegin(); rit != InChart->AudioPath.rend() && *rit != '/' && *rit != '\\'; ++rit)
		audioFileName.insert(audioFileName.begin(), *rit);
	
	std::stringstream chartStream;

	chartStream << "osu file format v14" << "\n"
						  << "\n"
						  << "[General]" << "\n"
						  << "AudioFilename: " << audioFileName << "\n"
						  << "AudioLeadIn: 0" << "\n"
						  << "PreviewTime: 0" << "\n"
						  << "Countdown: 0" << "\n"
						  << "SampleSet: Soft" << "\n"
						  << "StackLeniency: 0.7" << "\n"
						  << "Mode: 3" << "\n"
						  << "LetterboxInBreaks: 0" << "\n"
						  << "SpecialStyle: 0" << "\n"
						  << "WidescreenStoryboard: 0" << "\n"
						  << "\n"
						  << "[Editor]" << "\n"
						  << "DistanceSpacing: 1" << "\n"
						  << "BeatDivisor: 4" << "\n"
						  << "GridSize: 16" << "\n"
						  << "TimelineZoom: 1" << "\n"
						  << "\n"
						  << "[Metadata]" << "\n"
						  << "Title:" << InChart->SongTitle << "\n"
						  << "TitleUnicode:" << InChart->SongTitle << "\n"
						  << "Artist:" << InChart->Artist << "\n"
						  << "ArtistUnicode:" << InChart->Artist << "\n"
						  << "Creator:" << InChart->Charter << "\n"
						  << "Version:" << InChart->DifficultyName << "\n"
						  << "Source:" << "\n"
						  << "Tags:" << "\n"
						  << "BeatmapID:-1" << "\n"
						  << "BeatmapSetID:-1" << "\n"
						  << "\n"
						  << "[Difficulty]" << "\n"
						  << "HPDrainRate:8" << "\n"
						  << "CircleSize:" << InChart->KeyAmount << "\n"
						  << "OverallDifficulty:8" << "\n"
						  << "ApproachRate:9" << "\n"
						  << "SliderMultiplier:1.4" << "\n"
						  << "SliderTickRate:1" << "\n"
						  << "\n"
						  << "[Events]" << "\n"
							<< "//Background and Video events" << "\n";

	if (backgroundFileName != "")
		chartStream << "0,0,\"" << backgroundFileName << "\",0,0" << "\n";

	chartStream << "//Break Periods" << "\n"
							<< "//Storyboard Layer 0 (Background)" << "\n"
							<< "//Storyboard Layer 1 (Fail)" << "\n"
							<< "//Storyboard Layer 2 (Pass)" << "\n"
							<< "//Storyboard Layer 3 (Foreground)" << "\n"
							<< "//Storyboard Layer 4 (Overlay)" << "\n"
							<< "//Storyboard Sound Samples" << "\n"
							<< "\n"
							<< "[TimingPoints]" << "\n";

	for (auto BpmPoint : InChart->GetBpmPointsRelatedToTimeRange(0, InSongLength))
	{
		chartStream << BpmPoint->TimePoint << "," << BpmPoint->BeatLength << "," << "4" << ",0,0,10,1,0\n";
	}
	// leaving the "4" there since we will want to set custom snap divisor
	
	chartStream << "\n"
							<< "\n"
							<< "[HitObjects]" << "\n";

	int keyAmount = InChart->KeyAmount;
	InChart->IterateNotesInTimeRange(0, InSongLength, [InSongLength, keyAmount, &chartStream](const Note InOutNote, const Column InColumn)
	{
		int column = float(float((InColumn + 1)) * 512.f) / float(keyAmount) - (512.f / float(keyAmount) / 2.f);

		switch (InOutNote.Type)
		{
		case Note::EType::Common:
			chartStream << column << ",192," << InOutNote.TimePoint << ",1,0,0:0:0:0:\n";
			break;
		
		case Note::EType::HoldBegin:
			chartStream << column << ",192," << InOutNote.TimePoint << ",128,0," << InOutNote.TimePointEnd << ":0:0:0:0:\n";
			break;

		default:
			break;
		}
	});

	InOfStream.clear();
	InOfStream << chartStream.str();
	InOfStream.close();
}
void ChartParserModule::ExportChartStepmaniaImpl(Chart* InChart, const Time InSongLength, std::ofstream& InOfStream)
{
	return;
}
