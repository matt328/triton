#pragma once

namespace tr {

struct FileAlias {
  std::string alias;
  std::filesystem::path filePath;

  auto operator==(const FileAlias&) const -> bool = default;

  template <class T>
  void serialize(T& archive) {
    auto filePathStr = filePath.string();
    archive(alias, filePathStr);

    if constexpr (T::is_loading::value) {
      filePath = std::filesystem::path{filePathStr};
    }
  }
};

}
