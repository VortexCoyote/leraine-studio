#pragma once

#include "base/module.h"

#include <fstream>

class ChartParserModule : public Module
{
public:
	void SetCurrentChartPath(const std::string InPath);
	Chart* ParseAndGenerateChartSet(std::string _CurrentChartPath);
	void ExportChartSet(Chart* InChart, const Time InSongLength);

private:

	std::string _CurrentChartPath;

	Chart* ParseChartOsuImpl(std::ifstream& InIfstream, std::string InPath);
	Chart* ParseChartStepmaniaImpl(std::ifstream& InIfstream, std::string InPath);

	void ExportChartOsuImpl(Chart* InChart, const Time InSongLength, std::ofstream& InOfStream);
	void ExportChartStepmaniaImpl(Chart* InChart, const Time InSongLength, std::ofstream& InOfStream);
};