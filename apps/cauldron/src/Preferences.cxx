#include "Preferences.hpp"

namespace ed {

Preferences::Preferences(std::filesystem::path newPath) : path{std::move(newPath)} {
  if (!exists(path.parent_path())) {
    create_directories(path.parent_path());
  }

  // Create the file if it doesn't exist
  if (!exists(path)) {
    if (std::ofstream o{path, std::ios::binary}; o.is_open()) {
      cereal::BinaryOutputArchive output(o);
      output(preferencesData);
    } else {
      Log.warn("Failed to create preferences file");
    }
  }

  try {
    if (std::ifstream i{path, std::ios::binary}; i.is_open()) {
      cereal::BinaryInputArchive input(i);
      input(preferencesData);
      loaded = true;
    } else {
      Log.warn("Error reading application preferences from: {0}", path.string());
    }
  } catch (const std::exception& ex) {
    Log.warn("Error reading application preferences from: {0}, {1}", path.string(), ex.what());
  }
  Log.debug("Loaded preferences");
}

[[nodiscard]] auto Preferences::getRecentFile() const -> std::optional<std::string> {
  return preferencesData.recentFile;
}

[[nodiscard]] auto Preferences::getLastOpenDialogPath() const -> std::optional<std::string> {
  return preferencesData.lastOpenDialogPath;
}

auto Preferences::setRecentFile(std::string filename) -> void {
  auto lock = std::lock_guard(mtx);
  preferencesData.recentFile = std::move(filename);
  save();
}

auto Preferences::setLastOpenDialogPath(std::string path) -> void {
  auto lock = std::lock_guard(mtx);
  preferencesData.lastOpenDialogPath = std::move(path);
  save();
}

auto Preferences::getStringValue(std::string key) const -> std::optional<std::string> {
  if (preferencesData.stringMap.contains(key)) {
    return preferencesData.stringMap.at(key);
  }
  return std::nullopt;
}

auto Preferences::putStringValue(std::string key, std::string value) -> void {
  auto lock = std::lock_guard(mtx);
  preferencesData.stringMap[key] = std::move(value);
  save();
}

void Preferences::save() const {
  if (std::ofstream o{path, std::ios::binary}; o.is_open()) {
    cereal::BinaryOutputArchive output{o};
    output(preferencesData);
  } else {
    Log.warn("Error saving application preferences to file: {0}", path.string());
  }
}

}
