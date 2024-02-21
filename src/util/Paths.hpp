#pragma once

namespace Triton::Util::Paths {
   const std::filesystem::path RESOURCES = std::filesystem::current_path() / "assets";
   const std::filesystem::path SHADERS = RESOURCES / "shaders";
   const std::filesystem::path TEXTURES = RESOURCES / "textures";
   const std::filesystem::path MODELS = RESOURCES / "models";
}
