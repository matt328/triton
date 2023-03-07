#pragma once

#include <filesystem>

namespace Paths {
   const std::filesystem::path RESOURCES = std::filesystem::current_path() / "resources";
   const std::filesystem::path SHADERS = RESOURCES / "shaders";
}