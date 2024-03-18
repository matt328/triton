#include "ctx/ModelInfo.hpp"

#include "ctx/KtxImage.hpp"
#include "ctx/GltfHelper.hpp"

namespace tr::ctx {
   ModelInfo::ModelInfo(fastgltf::Asset&& asset, const std::filesystem::path& filePath)
       : asset{std::move(asset)}, loadedImages{gltf::parseImages(asset, filePath.parent_path())} {
   }

   ModelInfo::~ModelInfo(){};
}