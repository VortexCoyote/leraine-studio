#pragma once

#include "base/module.h"

#include <fstream>

#include "../structures/newchart-data.h"


/*
* TODO: see over what variables we can change to std::filesystem specifics and look over what data we can save in the chart itself
*/
class ChartParserModule : public Module
{
public:
	
	Chart* ParseAndGenerateChartSet(const std::string& InPath);
	void ExportChartSet(Chart* InChart);

	void SetCurrentChartPath(const std::string& InPath);
	void SetBackground(Chart* OutChart, const std::string& InPath);

	//this is for now osu impl only, in the future I'll make some template magic for which format is present
	std::string CreateNewChart(const NewChartData& InNewChartData); 

private:

	std::string _CurrentChartPath;

	Chart* ParseChartOsuImpl(std::ifstream& InIfstream, std::string InPath);
	Chart* ParseChartStepmaniaImpl(std::ifstream& InIfstream, std::string InPath);

	void ExportChartOsuImpl(Chart* InChart, std::ofstream& InOfStream);
	void ExportChartStepmaniaImpl(Chart* InChart, std::ofstream& InOfStream);
};