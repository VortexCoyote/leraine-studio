#pragma once

#include "base/module.h"

#include <fstream>

class ChartParserModule : public Module
{
public:

	Chart* ParseAndGenerateChartSet(std::string InPath);

private:

	Chart* ParseChartOsuImpl(std::ifstream& InIfstream, std::string InPath);
	Chart* ParseChartStepmaniaImpl(std::ifstream& InIfstream, std::string InPath);
};