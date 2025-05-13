#pragma once

#include <optional>
#include <utility>
namespace cereal {

template <class Archive>
void serialize(Archive& ar, std::filesystem::path& path) {
  std::string pathStr = path.string();
  ar(pathStr);
  if (Archive::is_loading::value) {
    path = std::filesystem::path(pathStr);
  }
}

}

namespace ed {

struct PropertiesData {
  std::filesystem::path recentFile{};
  std::filesystem::path lastOpenDialogPath{};
  std::unordered_map<std::string, std::string> mapData;

  template <class T>
  void serialize(T& archive) {
    archive(recentFile, lastOpenDialogPath, mapData);
  }
};

class Properties {
public:
  explicit Properties(std::filesystem::path filePath) : path(std::move(filePath)) {
    if (!exists(path.parent_path())) {
      create_directories(path.parent_path());
    }

    // Create the file if it doesn't exist
    if (!exists(path)) {
      if (std::ofstream o{path, std::ios::binary}; o.is_open()) {
        cereal::BinaryOutputArchive output(o);
        output(propertiesData);
      } else {
        Log.warn("Failed to create properties file");
      }
    }

    try {
      if (std::ifstream i{path, std::ios::binary}; i.is_open()) {
        cereal::BinaryInputArchive input(i);
        input(propertiesData);
        loaded = true;
      } else {
        Log.warn("Error reading application configuration file: {0}", path.string());
      }
    } catch (const std::exception& ex) {
      Log.warn("Error reading application configuration file: {0}, {1}", path.string(), ex.what());
    }
    Log.debug("Loaded properties");
  }

  ~Properties() = default;

  Properties(const Properties&) = delete;
  Properties(Properties&&) = delete;
  auto operator=(const Properties&) -> Properties& = delete;
  auto operator=(Properties&&) -> Properties& = delete;

  auto put(std::string_view key, std::string_view value) -> void {
    auto lock = std::lock_guard(mtx);
    propertiesData.mapData[key.data()] = value.data();
    save();
  }

  [[nodiscard]] auto get(std::string_view key) const -> std::optional<std::string> {
    if (propertiesData.mapData.contains(key.data())) {
      return std::make_optional(propertiesData.mapData.at(key.data()));
    }
    return std::nullopt;
  }

  [[nodiscard]] auto getRecentFile() const -> std::optional<std::filesystem::path> {
    if (propertiesData.recentFile.empty()) {
      return std::nullopt;
    }
    return std::optional{propertiesData.recentFile};
  }

  [[nodiscard]] auto getLastOpenDialogPath() const -> std::optional<std::filesystem::path> {
    if (propertiesData.lastOpenDialogPath.empty()) {
      return std::nullopt;
    }
    return std::optional{propertiesData.lastOpenDialogPath};
  }

  auto setRecentFile(const std::filesystem::path& value) {
    auto lock = std::lock_guard(mtx);
    propertiesData.recentFile = value;
    save();
  }

  auto setLastOpenDialogPath(const std::filesystem::path& value) {
    auto lock = std::lock_guard(mtx);
    propertiesData.lastOpenDialogPath = value;
    save();
  }

private:
  PropertiesData propertiesData;
  std::filesystem::path path;
  std::mutex mtx;
  bool loaded{};

  void save() const {
    if (std::ofstream o{path, std::ios::binary}; o.is_open()) {
      cereal::BinaryOutputArchive output{o};
      output(propertiesData);
    } else {
      Log.warn("Error saving application configuration file: {0}", path.string());
    }
  }
};
}

CEREAL_CLASS_VERSION(ed::PropertiesData, 1);
