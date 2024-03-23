#include "TextureManager.hpp"

#include "gfx/VkContext.hpp"
#include "gfx/vma_raii.hpp"

namespace tr::gfx::tx {
   TextureManager::TextureManager(const VkContext& context, const Allocator& allocator)
       : context{context}, allocator{allocator} {

      const auto bci = vk::BufferCreateInfo{.size = MaxImageSize,
                                            .usage = vk::BufferUsageFlagBits::eTransferSrc,
                                            .sharingMode = vk::SharingMode::eExclusive};
      const auto aci =
          vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eCpuOnly,
                                    .requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible |
                                                     vk::MemoryPropertyFlagBits::eHostCoherent};

      stagingBuffer = allocator.createBuffer(&bci, &aci, "TextureManager Staging Buffer");
   }

   TextureManager::~TextureManager() {
   }

   std::vector<TextureHandle> TextureManager::uploadTextures(
       const std::vector<util::KtxImage>& ktxImages) {
      auto handles = std::vector<TextureHandle>{};

      for (const auto& ktxImage : ktxImages) {
         const auto data = allocator.mapMemory(*stagingBuffer);
         memcpy(data, ktxImage.getTexture()->pData, ktxImage.getTexture()->dataSize);
         allocator.unmapMemory(*stagingBuffer);

         // Set up buffer copy regions for each mip level
         std::vector<vk::BufferImageCopy> bufferCopyRegions;
         // TODO: Learn about mipmaps, array layers and layers
         const auto numLevels = ktxImage.getTexture()->numLevels;
         const auto width = ktxImage.getTexture()->baseWidth;
         const auto height = ktxImage.getTexture()->baseHeight;
         const auto depth = ktxImage.getTexture()->baseDepth;
         const auto format = vk::Format(ktxImage.getTexture()->vkFormat);

         for (uint32_t i = 0; i < numLevels; i++) {
            vk::DeviceSize offset{0};
            const auto bcr = vk::BufferImageCopy{
                .bufferOffset = offset,
                .imageSubresource =
                    vk::ImageSubresourceLayers{.aspectMask = vk::ImageAspectFlagBits::eColor,
                                               .mipLevel = i,
                                               .baseArrayLayer = 0,
                                               .layerCount = 1},
                .imageExtent = vk::Extent3D{.width = std::max(1u, width >> i),
                                            .height = std::max(1u, height >> i),
                                            .depth = depth}};
            bufferCopyRegions.push_back(bcr);
         }
         auto imageCreateInfo = vk::ImageCreateInfo{
             .imageType = vk::ImageType::e2D,
             .format = format,
             .extent = {width, height, depth},
             .mipLevels = numLevels,
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

         const auto image =
             allocator.createImage(imageCreateInfo, imageAllocateCreateInfo, ktxImage.getName());

         const auto subresourceRange =
             vk::ImageSubresourceRange{.aspectMask = vk::ImageAspectFlagBits::eColor,
                                       .baseMipLevel = 0,
                                       .levelCount = numLevels,
                                       .layerCount = 1};
      }

      return handles;
   }

}