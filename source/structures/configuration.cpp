#include "configuration.h"

// true if loading successful, false if config.yaml isn't found
bool Configuration::Load()
{
  YAML::Node configFile;
  try {
    configFile = YAML::LoadFile("config.yaml");
  }
  catch (YAML::BadFile) {
    Save();
    return false;
  }

  if (configFile["SkinFolderPath"]) SkinFolderPath = configFile["SkinFolderPath"].as<std::string>();
  if (configFile["UsePitch"]) UsePitch = configFile["UsePitch"].as<bool>();
  if (configFile["ShowColumnLines"]) ShowColumnLines = configFile["ShowColumnLines"].as<bool>();
  if (configFile["ShowWaveform"]) ShowWaveform = configFile["ShowWaveform"].as<bool>();
  if (configFile["UseAutoTiming"]) UseAutoTiming = configFile["UseAutoTiming"].as<bool>();
  if (configFile["ShowColumnHeatmap"]) ShowColumnHeatmap = configFile["ShowColumnHeatmap"].as<bool>();
  return true;
}

void Configuration::Save()
{
  YAML::Emitter out;
  out << YAML::BeginMap;
  out << YAML::Key << "SkinFolderPath";
  out << YAML::Value << SkinFolderPath.string();
  out << YAML::Key << "UsePitch";
  out << YAML::Value << UsePitch;
  out << YAML::Key << "ShowColumnLines";
  out << YAML::Value << ShowColumnLines;
  out << YAML::Key << "ShowWaveform";
  out << YAML::Value << ShowWaveform;
  out << YAML::Key << "UseAutoTiming";
  out << YAML::Value << UseAutoTiming;
  out << YAML::Key << "ShowColumnHeatmap";
  out << YAML::Value << ShowColumnHeatmap;
  out << YAML::EndMap;

  std::ofstream configFile("config.yaml");

  configFile << out.c_str();

}