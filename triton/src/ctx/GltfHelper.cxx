#include "ctx/GltfHelper.hpp"

#include "ctx/KtxImage.hpp"

namespace tr::ctx::gltf {

   std::vector<util::KtxImage> parseImages(const fastgltf::Asset& asset,
                                           const std::filesystem::path& path) {
      std::vector<util::KtxImage> ktxImages{};

      for (auto& image : asset.images) {
         bool imageCreated{};
         const auto& x = std::get<fastgltf::sources::Array>(image.data);

         auto y = x.bytes;

         std::visit(
             fastgltf::visitor{
                 []([[maybe_unused]] auto& arg) {},
                 [&](fastgltf::sources::URI& filePath) {
                    if (filePath.fileByteOffset != 0) {
                       throw std::runtime_error("Offset found in image. We don't do that here.");
                    }
                    if (!filePath.uri.isLocalPath()) {
                       throw std::runtime_error("Nonlocal file. Einstien was wrong.");
                    }
                    const auto imagePath = path / std::filesystem::path{filePath.uri.path()};
                    try {
                       ktxImages.emplace_back(imagePath);
                       imageCreated = true;
                    } catch (const std::exception& ex) { Log::error << ex.what() << std::endl; }
                 },
                 [&](const fastgltf::sources::Array& vector) {
                    try {
                       ktxImages.emplace_back(vector.bytes.data(), vector.bytes.size());
                       imageCreated = true;
                    } catch (const std::exception& ex) { Log::error << ex.what() << std::endl; }
                 },
                 [&](fastgltf::sources::BufferView view) {
                    auto& bufferView = asset.bufferViews[view.bufferViewIndex];
                    auto& buffer = asset.buffers[bufferView.bufferIndex];
                    std::visit(fastgltf::visitor{[]([[maybe_unused]] auto& arg) {},
                                                 [&](fastgltf::sources::Vector vector) {
                                                    try {
                                                       ktxImages.emplace_back(vector.bytes.data(),
                                                                              vector.bytes.size());
                                                       imageCreated = true;
                                                    } catch (const std::exception& ex) {
                                                       Log::error << ex.what() << std::endl;
                                                    }
                                                 }},
                               buffer.data);
                 }},
             image.data);
         // If for whatever reason an image isn't created, add a white 1px image in its place
         if (!imageCreated) {
            Log::warn << "gltf file at " << path << " missing a texture" << std::endl;
            ktxImages.emplace_back();
         }
      }
      return ktxImages;
   }

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