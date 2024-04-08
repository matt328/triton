#include "ctx/GltfHelper.hpp"

namespace tr::ctx::gltf {

   vk::Filter extractFilter(fastgltf::Filter filter) {
      switch (filter) {
         case fastgltf::Filter::Nearest:
         case fastgltf::Filter::NearestMipMapNearest:
         case fastgltf::Filter::NearestMipMapLinear:
            return vk::Filter::eNearest;
            // linear samplers
         case fastgltf::Filter::Linear:
         case fastgltf::Filter::LinearMipMapNearest:
         case fastgltf::Filter::LinearMipMapLinear:
         default:
            return vk::Filter::eLinear;
      }
   }

   vk::SamplerMipmapMode extractMipmapMode(fastgltf::Filter filter) {
      switch (filter) {
         case fastgltf::Filter::NearestMipMapNearest:
         case fastgltf::Filter::LinearMipMapNearest:
            return vk::SamplerMipmapMode::eNearest;

         case fastgltf::Filter::NearestMipMapLinear:
         case fastgltf::Filter::LinearMipMapLinear:
         default:
            return vk::SamplerMipmapMode::eLinear;
      }
   }

   std::vector<vk::SamplerCreateInfo> parseSamplers(const fastgltf::Asset& asset) {
      auto samplers = std::vector<vk::SamplerCreateInfo>{};

      for (const auto& sampler : asset.samplers) {
         auto samplerCreateInfo = vk::SamplerCreateInfo{
             .magFilter = extractFilter(sampler.magFilter.value_or(fastgltf::Filter::Nearest)),
             .minFilter = extractFilter(sampler.minFilter.value_or(fastgltf::Filter::Nearest)),
             .mipmapMode = extractMipmapMode(sampler.minFilter.value_or(fastgltf::Filter::Nearest)),
             .minLod = 0,
             .maxLod = vk::LodClampNone,
         };
         samplers.push_back(samplerCreateInfo);
      }

      return samplers;
   }

}