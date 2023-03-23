#pragma once

#include <ktx.h>

#include <filesystem>

namespace Paths {
const std::filesystem::path RESOURCES = std::filesystem::current_path() / "resources";
const std::filesystem::path SHADERS = RESOURCES / "shaders";
const std::filesystem::path TEXTURES = RESOURCES / "textures";
const std::filesystem::path MODELS = RESOURCES / "models";
}  // namespace Paths

namespace Textures {
ktxResult loadKtxFile(const std::string_view& filename, ktxTexture** target);
}