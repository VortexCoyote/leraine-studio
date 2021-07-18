#pragma once

#include <string>

struct NewChartData
{
    std::string Artist = "";
    std::string SongTitle = "";
    std::string Charter = "";
    std::string DifficultyName = "";

    std::string ChartPath = "";
    std::string AudioPath = "";

    int KeyAmount = 4;
    float OD = 8.0f;
    float HP = 8.0f;
};