#include "Texture.hpp"

#include "ImmediateTransferContext.hpp"
#include "mem/Buffer.hpp"
#include "mem/Image.hpp"
#include "mem/Allocator.hpp"

namespace tr {

   Texture::Texture(const std::string_view& filename,
                    const Allocator& allocator,
                    const vk::raii::Device& device,
                    const ImmediateTransferContext& transferContext)
       : imageLayout{vk::ImageLayout::eShaderReadOnlyOptimal} {
      Log.debug("Creating Texture from file: {0}", filename.data());

      int width = 0;
      int height = 0;
      int channels = 0;

      stbi_uc* pixels = stbi_load(filename.data(), &width, &height, &channels, STBI_rgb_alpha);

      if (pixels == nullptr) {
         throw std::runtime_error("Failed to load texture");
      }

      initialize(pixels, width, height, STBI_rgb_alpha, allocator, device, transferContext);
   }

   Texture::Texture(const void* data,
                    const uint32_t width,
                    const uint32_t height,
                    const uint32_t channels,
                    const Allocator& allocator,
                    const vk::raii::Device& device,
                    const ImmediateTransferContext& transferContext)
       : imageLayout{vk::ImageLayout::eShaderReadOnlyOptimal} {
      initialize(data, width, height, channels, allocator, device, transferContext);
   }

   Texture::~Texture() { // NOLINT(*-use-equals-default)
   }

   void Texture::initialize(const void* data,
                            uint32_t width,
                            uint32_t height,
                            uint32_t channels,
                            const Allocator& allocator,
                            const vk::raii::Device& device,
                            const ImmediateTransferContext& transferContext,
                            const std::string_view& textureName) {

      vk::DeviceSize textureSize = width * height * channels;

      // Create a Staging Buffer
      const auto bufferCreateInfo =
          vk::BufferCreateInfo{.size = textureSize,
                               .usage = vk::BufferUsageFlagBits::eTransferSrc,
                               .sharingMode = vk::SharingMode::eExclusive};
      const auto allocationCreateInfo =
          vma::AllocationCreateInfo{.usage = vma::MemoryUsage::eCpuOnly,
                                    .requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible |
                                                     vk::MemoryPropertyFlagBits::eHostCoherent};

      const auto stagingBuffer =
          allocator.createBuffer(&bufferCreateInfo, &allocationCreateInfo, textureName);

      // Copy the texture data into the staging buffer
      auto* const bufferData = allocator.mapMemory(*stagingBuffer);
      memcpy(bufferData, data, textureSize);
      allocator.unmapMemory(*stagingBuffer);

      // Setup buffer copy regions for each mip level
      std::vector<vk::BufferImageCopy> bufferCopyRegions;

      // hack
      constexpr uint32_t mipLevels = 1;
      const auto uWidth = width;
      const auto uHeight = height;

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
      constexpr auto format = vk::Format::eR8G8B8A8Srgb;
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

      image = allocator.createImage(imageCreateInfo, imageAllocateCreateInfo, textureName);

      constexpr auto subresourceRange =
          vk::ImageSubresourceRange{.aspectMask = vk::ImageAspectFlagBits::eColor,
                                    .baseMipLevel = 0,
                                    .levelCount = mipLevels,
                                    .layerCount = 1};

      // Upload Image
      transferContext.submit([this, &stagingBuffer, &bufferCopyRegions, &subresourceRange](
                                 const vk::raii::CommandBuffer& cmd) {
         {
            const auto [dstBarrier, sourceStage, dstStage] =
                createTransitionBarrier(image->getImage(),
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
                               image->getImage(),
                               vk::ImageLayout::eTransferDstOptimal,
                               bufferCopyRegions);
         {
            const auto [dstBarrier, sourceStage, dstStage] =
                createTransitionBarrier(image->getImage(),
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

      constexpr auto samplerInfo =
          vk::SamplerCreateInfo{.magFilter = vk::Filter::eLinear,
                                .minFilter = vk::Filter::eLinear,
                                .mipmapMode = vk::SamplerMipmapMode::eLinear,
                                .addressModeU = vk::SamplerAddressMode::eRepeat,
                                .addressModeV = vk::SamplerAddressMode::eRepeat,
                                .addressModeW = vk::SamplerAddressMode::eRepeat,
                                .mipLodBias = 0.f,
                                .anisotropyEnable = VK_TRUE,
                                .maxAnisotropy = 1, // TODO(matt): look this up
                                .compareEnable = VK_FALSE,
                                .compareOp = vk::CompareOp::eAlways,
                                .minLod = 0.f,
                                .maxLod = 0.f,
                                .borderColor = vk::BorderColor::eIntOpaqueBlack,
                                .unnormalizedCoordinates = VK_FALSE};

      sampler = std::make_unique<vk::raii::Sampler>(device.createSampler(samplerInfo));

      constexpr auto range =
          vk::ImageSubresourceRange{.aspectMask = vk::ImageAspectFlagBits::eColor,
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

   auto Texture::createTransitionBarrier(const vk::Image& image,
                                         const vk::ImageLayout oldLayout,
                                         const vk::ImageLayout newLayout,
                                         const vk::ImageSubresourceRange& subresourceRange)
       -> TransitionBarrierInfo {
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
      return {barrier, sourceStage, destinationStage};
   }
}