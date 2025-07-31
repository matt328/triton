#pragma once

namespace bk {

class Preferences {
public:
  Preferences(std::filesystem::path newPath);
  ~Preferences() = default;

  Preferences(const Preferences&) = delete;
  Preferences(Preferences&&) = delete;
  auto operator=(const Preferences&) -> Preferences& = delete;
  auto operator=(Preferences&&) -> Preferences& = delete;

  auto put(const std::string& key, const std::string& value) -> void;

  template <typename T>
  auto get(const std::string& key) const -> std::optional<T> {
    if constexpr (std::is_same_v<T, std::string>) {
      auto it = preferencesData.stringMap.find(key);
      return it != preferencesData.stringMap.end() ? std::optional{it->second} : std::nullopt;
    }
  }

private:
  struct Data {
    std::unordered_map<std::string, std::string> stringMap;

    template <class T>
    void serialize(T& archive) {
      archive(stringMap);
    }
  };

  Data preferencesData{};

  std::filesystem::path path;
  std::mutex mtx;
  bool loaded{};

  auto save() const -> void;
};

}
