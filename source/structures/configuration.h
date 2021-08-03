#pragma once

#include "yaml-cpp/yaml.h"
#include <filesystem>
#include <fstream>
#include <array>

struct Configuration
{
  std::filesystem::path SkinFolderPath = "data/skins/default";

  bool UsePitch = true;
  bool ShowColumnLines = false;
  bool ShowWaveform = true;
  bool UseAutoTiming = false;
  bool ShowColumnHeatmap = false;

  const int RecentFilePathsMaxSize = 10;
  //FIFO, but needs to remove invalid paths on access (like if the files have moved)
  std::vector<std::string> RecentFilePaths;

  bool Load();
  void Save();
  void RegisterRecentFile(const std::string InPath);
};