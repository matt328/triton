#pragma once

namespace tr {

struct FileAlias {
  std::string alias;
  std::filesystem::path filePath;

  auto operator==(const FileAlias&) const -> bool = default;
};

}
