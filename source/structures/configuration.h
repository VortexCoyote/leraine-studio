#pragma once

#include "yaml-cpp/yaml.h"
#include <filesystem>
#include <fstream>

struct Configuration
{
  std::filesystem::path SkinFolderPath = "data/skins/default";

  bool UsePitch = true;
  bool ShowColumnLines = false;
  bool ShowWaveform = true;
  bool UseAutoTiming = false;
  bool ShowColumnHeatmap = false;

  bool Load();
  void Save();
};