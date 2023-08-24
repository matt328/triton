#include "Texture.hpp"

#include "Log.hpp"
#include "core/Utils.hpp"
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

using Core::Log;

Texture::Texture(const std::string_view& filename,
                 const vma::raii::Allocator& raiillocator,
                 const vk::raii::Device& device,
                 const ImmediateContext& graphicsContext,
                 const ImmediateContext& transferContext) :
    device(device) {
   Log::core->debug("Creating Texture from file: {}", filename.data());

   int width = 0, height = 0, channels = 0;
   const auto format = vk::Format::eR8G8B8A8Srgb;

   stbi_uc* pixels = stbi_load(filename.data(), &width, &height, &channels, STBI_rgb_alpha);

   if (!pixels) {
      throw std::runtime_error(fmt::format("Failed to load texture file: {}", filename.data()));
   }

   vk::DeviceSize textureSize = width * height * 4;
   void* pixelPtr = pixels;

   // Create a Staging Buffer
   const auto bufferCreateInfo =
       vk::BufferCreateInfo{.size = textureSize,
                            .usage = vk::BufferUsageFlagBits::eTransferSrc,
                            .sharingMode = vk::SharingMode::eExclusive};
   const auto allocationCreateInfo =
       vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eCpuOnly,
                                 .requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible |
                                                  vk::MemoryPropertyFlagBits::eHostCoherent};

   const auto stagingBuffer = raiillocator.createBuffer(&bufferCreateInfo, &allocationCreateInfo);

   // Copy the texture data into the staging buffer
   const auto data = raiillocator.mapMemory(*stagingBuffer);
   memcpy(data, pixelPtr, textureSize);
   raiillocator.unmapMemory(*stagingBuffer);

   stbi_image_free(pixels);

   // Setup buffer copy regions for each mip level
   std::vector<vk::BufferImageCopy> bufferCopyRegions;

   // hack
   const uint32_t mipLevels = 1;
   const auto uWidth = static_cast<uint32_t>(width);
   const auto uHeight = static_cast<uint32_t>(height);

   for (uint32_t i = 0; i < mipLevels; i++) {
      vk::DeviceSize offset{0};
      // ktx_size_t offset;
      // if (const auto result = ktxTexture_GetImageOffset(ktxTexture, i, 0, 0, &offset);
      //     result != KTX_SUCCESS) {
      //    throw new std::runtime_error(
      //        std::format("Error getting image offset for mip level {}", i));
      // }

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

   image = raiillocator.createImage(imageCreateInfo, imageAllocateCreateInfo);

   const auto subresourceRange =
       vk::ImageSubresourceRange{.aspectMask = vk::ImageAspectFlagBits::eColor,
                                 .baseMipLevel = 0,
                                 .levelCount = mipLevels,
                                 .layerCount = 1};
   // TODO Change these utils so they can happen in one 'transaction' without flushing the command
   // buffer after each one.  Immediate context could for allow for begin, then add function, then
   // end maybe.
   transitionImageLayout(graphicsContext,
                         image->getImage(),
                         vk::ImageLayout::eUndefined,
                         vk::ImageLayout::eTransferDstOptimal,
                         subresourceRange);

   copyBufferToImage(transferContext,
                     stagingBuffer->getBuffer(),
                     image->getImage(),
                     vk::ImageLayout::eTransferDstOptimal,
                     bufferCopyRegions);

   transitionImageLayout(
       graphicsContext,
       image->getImage(),
       vk::ImageLayout::eTransferDstOptimal,
       vk::ImageLayout::eShaderReadOnlyOptimal, // TODO: pass this in to the function, but default
                                                // it to this.
       subresourceRange);

   imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

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

   sampler = std::make_unique<vk::raii::Sampler>(device.createSampler(samplerInfo));

   constexpr auto range = vk::ImageSubresourceRange{.aspectMask = vk::ImageAspectFlagBits::eColor,
                                                    .baseMipLevel = 0,
                                                    .levelCount = 1,
                                                    .baseArrayLayer = 0,
                                                    .layerCount = 1};

   const auto viewInfo = vk::ImageViewCreateInfo{.image = image->getImage(),
                                                 .viewType = vk::ImageViewType::e2D,
                                                 .format = format,
                                                 .subresourceRange = range};

   view = std::make_unique<vk::raii::ImageView>(device.createImageView(viewInfo));

   imageInfo = vk::DescriptorImageInfo{
       .sampler = **sampler,
       .imageView = **view,
       .imageLayout = imageLayout,
   };
}

void Texture::updateDescriptorSet(const vk::raii::DescriptorSet& descriptorSet) const {
   const auto imageInfo = vk::DescriptorImageInfo{
       .sampler = **sampler,
       .imageView = **view,
       .imageLayout = imageLayout,
   };
   const auto textureWrite = std::array{vk::WriteDescriptorSet{
       .dstSet = *descriptorSet,
       .dstBinding = 1,
       .dstArrayElement = 0,
       .descriptorCount = 1,
       .descriptorType = vk::DescriptorType::eCombinedImageSampler,
       .pImageInfo = &imageInfo,
   }};
   const auto writes = std::array{textureWrite};
   device.updateDescriptorSets(writes, nullptr);
}

void Texture::transitionImageLayout(const ImmediateContext& context,
                                    const vk::Image& image,
                                    const vk::ImageLayout oldLayout,
                                    const vk::ImageLayout newLayout,
                                    const vk::ImageSubresourceRange subresourceRange) {
   const auto transitionImageLayoutFn = [&](const vk::raii::CommandBuffer& cmd) {
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

      cmd.pipelineBarrier(
          sourceStage, destinationStage, vk::DependencyFlagBits::eByRegion, {}, {}, barrier);
   };

   context.submit(transitionImageLayoutFn);
}

void Texture::copyBufferToImage(const ImmediateContext& context,
                                const vk::Buffer& buffer,
                                const vk::Image& image,
                                const vk::ImageLayout imageLayout,
                                const std::vector<vk::BufferImageCopy>& regions) {

   const auto copyBufferToImageFn = [&](const vk::raii::CommandBuffer& cmd) {
      cmd.copyBufferToImage(buffer, image, imageLayout, regions);
   };

   context.submit(copyBufferToImageFn);
}
