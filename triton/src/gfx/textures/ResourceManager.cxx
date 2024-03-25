#include "ResourceManager.hpp"

#include "gfx/VkContext.hpp"
#include "gfx/vma_raii.hpp"
#include "gfx/GraphicsDevice.hpp"
#include <fastgltf/types.hpp>

#include "ctx/GltfHelper.hpp"

namespace tr::gfx::tx {
   ResourceManager::ResourceManager(const GraphicsDevice& graphicsDevice)
       : graphicsDevice{graphicsDevice} {

      const auto bci = vk::BufferCreateInfo{.size = MaxImageSize,
                                            .usage = vk::BufferUsageFlagBits::eTransferSrc,
                                            .sharingMode = vk::SharingMode::eExclusive};
      const auto aci =
          vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eCpuOnly,
                                    .requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible |
                                                     vk::MemoryPropertyFlagBits::eHostCoherent};

      stagingBuffer =
          graphicsDevice.getAllocator().createBuffer(&bci, &aci, "ResourceManager Staging Buffer");
   }

   ResourceManager::~ResourceManager() {
   }

   ModelHandle ResourceManager::loadModel(const std::filesystem::path& filename) {
      static constexpr auto supportedExtensions = fastgltf::Extensions::KHR_mesh_quantization |
                                                  fastgltf::Extensions::KHR_texture_basisu |
                                                  fastgltf::Extensions::KHR_texture_transform;

      fastgltf::Parser parser{supportedExtensions};

      constexpr auto gltfOptions =
          fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::AllowDouble |
          fastgltf::Options::LoadGLBBuffers | fastgltf::Options::LoadExternalBuffers |
          fastgltf::Options::LoadExternalImages | fastgltf::Options::GenerateMeshIndices;

      fastgltf::GltfDataBuffer data{};
      data.loadFromFile(filename);

      auto asset = parser.loadGltf(&data, filename.parent_path(), gltfOptions);
      if (asset.error() != fastgltf::Error::None) {
         Log::error << "Failed to load glTF: " << fastgltf::getErrorMessage(asset.error())
                    << std::endl;
         throw std::runtime_error("Failed to load glTF");
      }

      auto materials = std::set<size_t>{};
      auto textures = std::set<size_t>{};
      auto samplers = std::set<size_t>{};

      Log::info << "textures size: " << asset->textures.size() << std::endl;
      Log::info << "materials size: " << asset->materials.size() << std::endl;

      auto loadedTextureIndices = std::unordered_map<std::size_t, TextureHandle>{};

      auto modelHandle = ModelHandle{};

      for (const auto& scene : asset->scenes) {
         for (const auto& nodeIndex : scene.nodeIndices) {

            const auto& node = asset->nodes[nodeIndex];
            const auto& mesh = asset->meshes[node.meshIndex.value()];

            for (const auto& primitive : mesh.primitives) {

               const auto meshHandle = createMesh(asset.get(), primitive);
               auto textureHandle = TextureHandle{}; // Init this to a debug texture

               const auto materialIndex = primitive.materialIndex;

               if (materialIndex.has_value()) {
                  const auto& material = asset->materials[materialIndex.value()];
                  const auto& baseColorTexture = material.pbrData.baseColorTexture;

                  if (baseColorTexture.has_value()) {
                     auto it = loadedTextureIndices.find(baseColorTexture.value().textureIndex);
                     if (it != loadedTextureIndices.end()) {
                        textureHandle = it->second;
                     } else {
                        textureHandle = createTexture(asset.get(),
                                                      baseColorTexture.value().textureIndex,
                                                      filename.parent_path());
                        loadedTextureIndices.insert(
                            {baseColorTexture.value().textureIndex, textureHandle});
                     }
                  }
               }
               modelHandle.insert({meshHandle, textureHandle});
            }
         }
      }
      return modelHandle;
   }

   /// Could factor out the gltf specific pieces from the vulkan specific pieces.
   TextureHandle ResourceManager::createTexture(const fastgltf::Asset& asset,
                                                std::size_t textureIndex,
                                                const std::filesystem::path& folder) {
      const auto texture = asset.textures[textureIndex];
      auto image = fastgltf::Image{};

      if (texture.imageIndex.has_value()) {
         const auto image = asset.images[texture.imageIndex.value()];
      } else {
         Log::info << "unsupported image type found for texture" << std::endl;
         // Return default texture handle here
         return 0;
      }

      unsigned char* data = nullptr;
      int width{}, height{}, numChannels{};

      std::visit(fastgltf::visitor{
                     []([[maybe_unused]] auto& arg) {},
                     [&](fastgltf::sources::URI& filePath) {
                        if (filePath.fileByteOffset != 0) {
                           throw std::runtime_error(
                               "Offset found in image. We don't do that here.");
                        }
                        if (!filePath.uri.isLocalPath()) {
                           throw std::runtime_error("Nonlocal file. Einstien was wrong.");
                        }
                        const auto imagePath = folder / std::filesystem::path{filePath.uri.path()};
                        data = stbi_load(imagePath.string().c_str(),
                                         &width,
                                         &height,
                                         &numChannels,
                                         STBI_rgb_alpha);
                     },
                     [&](const fastgltf::sources::Array& vector) {
                        data = stbi_load_from_memory(vector.bytes.data(),
                                                     static_cast<int>(vector.bytes.size()),
                                                     &width,
                                                     &height,
                                                     &numChannels,
                                                     STBI_rgb_alpha);
                     },
                     [&](fastgltf::sources::BufferView view) {
                        auto& bufferView = asset.bufferViews[view.bufferViewIndex];
                        auto& buffer = asset.buffers[bufferView.bufferIndex];
                        std::visit(fastgltf::visitor{[]([[maybe_unused]] auto& arg) {},
                                                     [&](fastgltf::sources::Vector vector) {
                                                        data = stbi_load_from_memory(
                                                            vector.bytes.data(),
                                                            static_cast<int>(vector.bytes.size()),
                                                            &width,
                                                            &height,
                                                            &numChannels,
                                                            STBI_rgb_alpha);
                                                     }},
                                   buffer.data);
                     }},
                 image.data);
      if (data == nullptr) {
         Log::warn << "Couldn't find a texture in glft file" << std::endl;
         // Skip loading and just return the handle of the default texture
         data[0] = 255;
         width = 1;
         height = 1;
         numChannels = 4;
      }

      const auto allocator = graphicsDevice.getAllocator();

      const auto bufferData = allocator.mapMemory(*stagingBuffer);
      memcpy(bufferData, data, width * height * numChannels);
      allocator.unmapMemory(*stagingBuffer);

      stbi_image_free(data);

      // Setup buffer copy regions for each mip level
      std::vector<vk::BufferImageCopy> bufferCopyRegions;

      const uint32_t mipLevels = 1;
      const auto uWidth = static_cast<uint32_t>(width);
      const auto uHeight = static_cast<uint32_t>(height);

      for (uint32_t i = 0; i < mipLevels; i++) {
         vk::DeviceSize offset{0};

         const auto bufferCopyRegion =
             vk::BufferImageCopy{.bufferOffset = offset,
                                 .imageSubresource =
                                     vk::ImageSubresourceLayers{
                                         .aspectMask = vk::ImageAspectFlagBits::eColor,
                                         .mipLevel = i,
                                         .baseArrayLayer = 0,
                                         .layerCount = 1,
                                     },
                                 .imageExtent = vk::Extent3D{.width = std::max(1u, uWidth >> i),
                                                             .height = std::max(1u, uHeight >> i),
                                                             .depth = 1}};
         bufferCopyRegions.push_back(bufferCopyRegion);
      }

      const auto format = vk::Format::eR8G8B8A8Srgb;

      auto imageCreateInfo = vk::ImageCreateInfo{
          .imageType = vk::ImageType::e2D,
          .format = format,
          .extent = {uWidth, uHeight, 1},
          .mipLevels = mipLevels,
          .arrayLayers = 1,
          .samples = vk::SampleCountFlagBits::e1,
          .tiling = vk::ImageTiling::eOptimal,
          .usage = vk::ImageUsageFlagBits::eSampled,
          .sharingMode = vk::SharingMode::eExclusive,
          .initialLayout = vk::ImageLayout::eUndefined,
      };
      if (!(imageCreateInfo.usage & vk::ImageUsageFlagBits::eTransferDst)) {
         imageCreateInfo.usage |= vk::ImageUsageFlagBits::eTransferDst;
      }

      constexpr auto imageAllocateCreateInfo =
          vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eGpuOnly,
                                    .requiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal};

      const auto imageName = "image - " + image.name;

      const auto pos = textureList.size();

      textureList.push_back({});

      textureList[pos].image =
          allocator.createImage(imageCreateInfo, imageAllocateCreateInfo, imageName);

      const auto subresourceRange =
          vk::ImageSubresourceRange{.aspectMask = vk::ImageAspectFlagBits::eColor,
                                    .baseMipLevel = 0,
                                    .levelCount = mipLevels,
                                    .layerCount = 1};

      const auto& context = graphicsDevice.getAsyncTransferContext();

      // Upload Image
      context.submit(
          [this, &pos, &bufferCopyRegions, &subresourceRange](const vk::raii::CommandBuffer& cmd) {
             {
                const auto [dstBarrier, sourceStage, dstStage] =
                    createTransitionBarrier(textureList[pos].image->getImage(),
                                            vk::ImageLayout::eUndefined,
                                            vk::ImageLayout::eTransferDstOptimal,
                                            subresourceRange);
                cmd.pipelineBarrier(sourceStage,
                                    dstStage,
                                    vk::DependencyFlagBits::eByRegion,
                                    {},
                                    {},
                                    dstBarrier);
             }
             cmd.copyBufferToImage(stagingBuffer->getBuffer(),
                                   textureList[pos].image->getImage(),
                                   vk::ImageLayout::eTransferDstOptimal,
                                   bufferCopyRegions);
             {
                const auto [dstBarrier, sourceStage, dstStage] =
                    createTransitionBarrier(textureList[pos].image->getImage(),
                                            vk::ImageLayout::eTransferDstOptimal,
                                            vk::ImageLayout::eShaderReadOnlyOptimal,
                                            subresourceRange);
                cmd.pipelineBarrier(sourceStage,
                                    dstStage,
                                    vk::DependencyFlagBits::eByRegion,
                                    {},
                                    {},
                                    dstBarrier);
             }
          });

      // Sampler
      auto sci = DefaultSamplerInfo;
      if (texture.samplerIndex.has_value()) {
         const auto sampler = asset.samplers[texture.samplerIndex.value()];
         sci = vk::SamplerCreateInfo{
             .magFilter =
                 ctx::gltf::extractFilter(sampler.magFilter.value_or(fastgltf::Filter::Nearest)),
             .minFilter =
                 ctx::gltf::extractFilter(sampler.minFilter.value_or(fastgltf::Filter::Nearest)),
             .mipmapMode = ctx::gltf::extractMipmapMode(
                 sampler.minFilter.value_or(fastgltf::Filter::Nearest)),
             .minLod = 0,
             .maxLod = vk::LodClampNone,
         };
      }
      textureList[pos].sampler =
          std::make_unique<vk::raii::Sampler>(graphicsDevice.getVulkanDevice().createSampler(sci));

      constexpr auto range =
          vk::ImageSubresourceRange{.aspectMask = vk::ImageAspectFlagBits::eColor,
                                    .baseMipLevel = 0,
                                    .levelCount = 1,
                                    .baseArrayLayer = 0,
                                    .layerCount = 1};

      const auto viewInfo = vk::ImageViewCreateInfo{.image = textureList[pos].image->getImage(),
                                                    .viewType = vk::ImageViewType::e2D,
                                                    .format = format,
                                                    .subresourceRange = range};

      textureList[pos].imageView = std::make_unique<vk::raii::ImageView>(
          graphicsDevice.getVulkanDevice().createImageView(viewInfo));

      textureList[pos].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

      return static_cast<TextureHandle>(pos);
   }

   MeshHandle ResourceManager::createMesh(const fastgltf::Asset& asset,
                                          const fastgltf::Primitive& primitive) {
   }

   const TransitionBarrierInfo ResourceManager::createTransitionBarrier(
       const vk::Image& image,
       const vk::ImageLayout oldLayout,
       const vk::ImageLayout newLayout,
       const vk::ImageSubresourceRange subresourceRange) {
      vk::ImageMemoryBarrier barrier{
          .srcAccessMask = vk::AccessFlagBits::eNone,
          .dstAccessMask = vk::AccessFlagBits::eNone,
          .oldLayout = oldLayout,
          .newLayout = newLayout,
          .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
          .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
          .image = image,
          .subresourceRange = subresourceRange,
      };

      auto sourceStage = vk::PipelineStageFlagBits::eNone;
      auto destinationStage = vk::PipelineStageFlagBits::eNone;

      if (oldLayout == vk::ImageLayout::eUndefined &&
          newLayout == vk::ImageLayout::eTransferDstOptimal) {
         barrier.srcAccessMask = vk::AccessFlagBits::eNone;
         barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
         sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
         destinationStage = vk::PipelineStageFlagBits::eTransfer;
      } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
                 newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
         barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
         barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
         sourceStage = vk::PipelineStageFlagBits::eTransfer;
         destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
      } else {
         throw std::invalid_argument("Unsupported layout transition");
      }
      return TransitionBarrierInfo(barrier, sourceStage, destinationStage);
   }
}