#pragma once

#include <filesystem>
#include <ktx.h>

namespace Paths {
   const std::filesystem::path RESOURCES = std::filesystem::current_path() / "resources";
   const std::filesystem::path SHADERS = RESOURCES / "shaders";
   const std::filesystem::path TEXTURES = RESOURCES / "textures";
}

namespace Textures {
   ktxResult loadKtxFile(const std::string_view& filename, ktxTexture** target);
}