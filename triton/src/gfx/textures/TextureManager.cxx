#include "TextureManager.hpp"

#include "gfx/VkContext.hpp"
#include "gfx/vma_raii.hpp"
#include "gfx/GraphicsDevice.hpp"
#include <vulkan/vulkan_enums.hpp>

namespace tr::gfx::tx {
   TextureManager::TextureManager(const GraphicsDevice& graphicsDevice)
       : graphicsDevice{graphicsDevice} {

      const auto bci = vk::BufferCreateInfo{.size = MaxImageSize,
                                            .usage = vk::BufferUsageFlagBits::eTransferSrc,
                                            .sharingMode = vk::SharingMode::eExclusive};
      const auto aci =
          vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eCpuOnly,
                                    .requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible |
                                                     vk::MemoryPropertyFlagBits::eHostCoherent};

      stagingBuffer =
          graphicsDevice.getAllocator().createBuffer(&bci, &aci, "TextureManager Staging Buffer");
   }

   TextureManager::~TextureManager() {
   }

   // TODO: Pass in the SamplerCreateInfos
   std::vector<TextureHandle> TextureManager::uploadTextures(
       const std::vector<util::KtxImage>& ktxImages,
       const std::vector<vk::SamplerCreateInfo>& samplerInfo) {
      auto handles = std::vector<TextureHandle>{};

      const auto allocator = graphicsDevice.getAllocator();
      const auto& context = graphicsDevice.getAsyncTransferContext();

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
         // const auto format = vk::Format(ktxImage.getTexture()->vkFormat);

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
             .format = vk::Format::eR8G8B8A8Srgb,
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

         const auto pos = imageInfoList.size();
         imageInfoList.emplace_back();

         imageInfoList[pos].image =
             allocator.createImage(imageCreateInfo, imageAllocateCreateInfo, ktxImage.getName());

         imageInfoList[pos].subresourceRange.emplace(
             vk::ImageSubresourceRange{.aspectMask = vk::ImageAspectFlagBits::eColor,
                                       .baseMipLevel = 0,
                                       .levelCount = numLevels,
                                       .layerCount = 1});

         context.submit([this, &pos, &bufferCopyRegions](const vk::raii::CommandBuffer& cmd) {
            {
               const auto [dstBarrier, sourceStage, dstStage] =
                   createTransitionBarrier(imageInfoList[pos].image->getImage(),
                                           vk::ImageLayout::eUndefined,
                                           vk::ImageLayout::eTransferDstOptimal,
                                           imageInfoList[pos].subresourceRange.value());
               cmd.pipelineBarrier(sourceStage,
                                   dstStage,
                                   vk::DependencyFlagBits::eByRegion,
                                   {},
                                   {},
                                   dstBarrier);
            }
            cmd.copyBufferToImage(stagingBuffer->getBuffer(),
                                  imageInfoList[pos].image->getImage(),
                                  vk::ImageLayout::eTransferDstOptimal,
                                  bufferCopyRegions);
            {
               const auto [dstBarrier, sourceStage, dstStage] =
                   createTransitionBarrier(imageInfoList[pos].image->getImage(),
                                           vk::ImageLayout::eTransferDstOptimal,
                                           vk::ImageLayout::eShaderReadOnlyOptimal,
                                           imageInfoList[pos].subresourceRange.value());
               cmd.pipelineBarrier(sourceStage,
                                   dstStage,
                                   vk::DependencyFlagBits::eByRegion,
                                   {},
                                   {},
                                   dstBarrier);
            }
         });
         constexpr auto samplerInfo =
             vk::SamplerCreateInfo{.magFilter = vk::Filter::eLinear,
                                   .minFilter = vk::Filter::eLinear,
                                   .mipmapMode = vk::SamplerMipmapMode::eLinear,
                                   .addressModeU = vk::SamplerAddressMode::eRepeat,
                                   .addressModeV = vk::SamplerAddressMode::eRepeat,
                                   .addressModeW = vk::SamplerAddressMode::eRepeat,
                                   .mipLodBias = 0.f,
                                   .anisotropyEnable = VK_TRUE,
                                   .maxAnisotropy = 1, // TODO: look this up
                                   .compareEnable = VK_FALSE,
                                   .compareOp = vk::CompareOp::eAlways,
                                   .minLod = 0.f,
                                   .maxLod = 0.f,
                                   .borderColor = vk::BorderColor::eIntOpaqueBlack,
                                   .unnormalizedCoordinates = VK_FALSE};

         imageInfoList[pos].sampler = std::make_unique<vk::raii::Sampler>(
             graphicsDevice.getVulkanDevice().createSampler(samplerInfo));
      }

      // create image view

      return handles;
   }

   const TransitionBarrierInfo TextureManager::createTransitionBarrier(
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