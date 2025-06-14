#pragma once

#include <filesystem>

namespace tr {
inline auto getShaderRootPath() -> const std::filesystem::path& {
  static const std::filesystem::path path = std::filesystem::current_path() / "assets" / "shaders";
  return path;
}
}
