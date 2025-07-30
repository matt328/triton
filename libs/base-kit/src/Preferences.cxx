#include "bk/Preferences.hpp"

namespace bk {

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

auto Preferences::put(const std::string& key, const std::string& value) -> void {
  auto lock = std::lock_guard(mtx);
  preferencesData.stringMap[key] = value;
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
