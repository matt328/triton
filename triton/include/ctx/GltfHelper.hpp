#pragma once

namespace tr::util {
   class KtxImage;
}

namespace tr::ctx::gltf {
   std::vector<util::KtxImage> parseImages(const fastgltf::Asset& asset,
                                           const std::filesystem::path& path);

   vk::Filter extractFilter(fastgltf::Filter filter);

   vk::SamplerMipmapMode extractMipmapMode(fastgltf::Filter filter);

   std::vector<vk::SamplerCreateInfo> parseSamplers(const fastgltf::Asset& asset);
}