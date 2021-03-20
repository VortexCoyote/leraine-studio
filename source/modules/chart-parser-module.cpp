#include "chart-parser-module.h"

#include <filesystem>
#include <sstream>
#include <algorithm>

#include <cmath>

namespace 
{
	auto getLine = [](std::ifstream& InIfstream, std::string& OutLine) -> auto&
		{
			auto& result = std::getline(InIfstream, OutLine); 
			OutLine.erase(remove(OutLine.begin(), OutLine.end(), '\r'), OutLine.end()); 
			
			return result; 
		}; 
};

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
	
	while (getLine(InIfstream, line))
	{
		if (line == "[General]")
		{
			std::string metadataLine;

			while (getLine(InIfstream, metadataLine))
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
					std::string songPath = parentPath + "/" + value;
					songPath.erase(remove(songPath.begin(), songPath.end(), '\r'), songPath.end()); 

					chart->AudioPath = songPath;
					//chartData->audioFileName = value;
				}
			}
		}

		if (line == "[Metadata]")
		{
			std::string metadataLine;

			while (getLine(InIfstream, metadataLine))
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

			while (getLine(InIfstream, difficultyLine))
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

			while (getLine(InIfstream, timePointLine))
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

				float timePoint, bpm;
				int meter, sampleSet, sampleIndex, volume, uninherited, effects;

				PARSE_COMMA_VALUE(timePointStream, timePoint);
				PARSE_COMMA_VALUE(timePointStream, bpm);
				PARSE_COMMA_VALUE(timePointStream, meter);
				PARSE_COMMA_VALUE(timePointStream, sampleSet);
				PARSE_COMMA_VALUE(timePointStream, sampleIndex);
				PARSE_COMMA_VALUE(timePointStream, volume);
				PARSE_COMMA_VALUE(timePointStream, uninherited);
				PARSE_COMMA_VALUE(timePointStream, effects);

				//BPMData* bpmData = new BPMData();
				//bpmData->BPMSaved = bpm;

				if (bpm < 0)
					continue;

				bpm = 60000.0 / bpm;

				//bpmData->BPM = bpm;
				//bpmData->timePoint = int(timePoint);
				//bpmData->meter = meter;
				//bpmData->uninherited = uninherited;

				chart->InjectBpmPoint(Time(timePoint), bpm);
			}
		}

		if (line == "[HitObjects]")
		{
			std::string noteLine;
			while (getLine(InIfstream, noteLine))
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

				int parsedColumn = std::clamp(float(floor(float(column) * (float(chart->KeyAmount) / 512.f))), 0.f, float(chart->KeyAmount) - 1.f);

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
