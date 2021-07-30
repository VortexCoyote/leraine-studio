#pragma once

#include <string>

struct ChartMetadata
{
    std::string Artist = "";
    std::string SongTitle = "";
    std::string Charter = "";
    std::string DifficultyName = "";

    std::string ChartFolderPath = "";
    std::string AudioPath = "";
    std::string BackgroundPath = "";

    int KeyAmount = 4;
    float OD = 8.0f;
    float HP = 8.0f;
};