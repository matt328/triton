#pragma once

namespace tr::util {
   class KtxImage;
}

namespace tr::ctx::gltf {

   vk::Filter extractFilter(fastgltf::Filter filter);

   vk::SamplerMipmapMode extractMipmapMode(fastgltf::Filter filter);

   std::vector<vk::SamplerCreateInfo> parseSamplers(const fastgltf::Asset& asset);
}