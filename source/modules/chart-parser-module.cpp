#include "chart-parser-module.h"

#include <filesystem>
#include <sstream>
#include <algorithm>

#define PARSE_COMMA_VALUE(stringstream, target) stringstream >> target; if (stringstream.peek() == ',') stringstream.ignore()

Chart* ChartParserModule::ParseAndGenerateChartSet(std::string InPath)
{
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

				Time timePoint;
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
