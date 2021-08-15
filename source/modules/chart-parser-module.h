#pragma once

#include "base/module.h"

#include <fstream>
#include <filesystem>

#include "../structures/chart-metadata.h"


/*
* TODO: see over what variables we can change to std::filesystem specifics and look over what data we can save in the chart itself
*/
class ChartParserModule : public Module
{
public:
	
	Chart* ParseAndGenerateChartSet(const std::filesystem::path& InPath);
	void ExportChartSet(Chart* InChart);
	
	void SetCurrentChartPath(const std::filesystem::path& InPath);

	ChartMetadata GetChartMetadata(Chart* InChart);

	//this is for now osu impl only, in the future I'll make some template magic for which format is present
	std::string SetChartMetadata(Chart* Outchart, const ChartMetadata& InMetadata);
	std::string CreateNewChart(const ChartMetadata& InNewChartData);
private:

	std::filesystem::path _CurrentChartPath;

	Chart* ParseChartOsuImpl(std::ifstream& InIfstream, std::filesystem::path InPath);
	Chart* ParseChartStepmaniaImpl(std::ifstream& InIfstream, std::filesystem::path InPath);

	void ExportChartOsuImpl(Chart* InChart, std::ofstream& InOfStream);
	void ExportChartStepmaniaImpl(Chart* InChart, std::ofstream& InOfStream);
};