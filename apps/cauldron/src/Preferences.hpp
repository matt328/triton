#pragma once

namespace ed {

class Preferences {
public:
  Preferences(std::filesystem::path newPath);
  ~Preferences() = default;

  Preferences(const Preferences&) = delete;
  Preferences(Preferences&&) = delete;
  auto operator=(const Preferences&) -> Preferences& = delete;
  auto operator=(Preferences&&) -> Preferences& = delete;

  [[nodiscard]] auto getRecentFile() const -> std::optional<std::string>;
  [[nodiscard]] auto getLastOpenDialogPath() const -> std::optional<std::string>;
  [[nodiscard]] auto getStringValue(std::string key) const -> std::optional<std::string>;

  auto setRecentFile(std::string filename) -> void;
  auto setLastOpenDialogPath(std::string path) -> void;
  auto putStringValue(std::string key, std::string value) -> void;

private:
  struct Data {
    std::optional<std::string> recentFile;
    std::optional<std::string> lastOpenDialogPath;

    std::unordered_map<std::string, std::string> stringMap;

    template <class T>
    void serialize(T& archive) {
      archive(recentFile, lastOpenDialogPath, stringMap);
    }
  };

  Data preferencesData{};

  std::filesystem::path path;
  std::mutex mtx;
  bool loaded{};

  auto save() const -> void;
};

}
