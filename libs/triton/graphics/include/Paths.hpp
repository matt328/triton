#pragma once

namespace tr::util::Paths {
   const std::filesystem::path RESOURCES = std::filesystem::current_path() / "assets";
   const std::filesystem::path SHADERS = RESOURCES / "shaders";
   const std::filesystem::path TEXTURES = RESOURCES / "textures";
}
