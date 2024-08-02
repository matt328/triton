#pragma once

namespace tr::gfx::Helpers {
   /// Maybe could make this a little more specific without having 15 params
   inline void transitionImage(const vk::raii::CommandBuffer& cmd,
                               const vk::Image& image,
                               const vk::ImageLayout currentLayout,
                               const vk::ImageLayout newLayout) {
      const auto barrier = vk::ImageMemoryBarrier{
          .srcAccessMask = vk::AccessFlagBits::eMemoryWrite,
          .dstAccessMask = vk::AccessFlagBits::eMemoryWrite | vk::AccessFlagBits::eMemoryRead,
          .oldLayout = currentLayout,
          .newLayout = newLayout,
          .image = image,
          .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                               .levelCount = 1,
                               .layerCount = 1}};

      cmd.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,
                          vk::PipelineStageFlagBits::eAllCommands,
                          vk::DependencyFlagBits{}, // None
                          {},
                          {},
                          barrier);
   }

   inline void copyImageToImage(const vk::raii::CommandBuffer& cmd,
                                const vk::Image source,
                                const vk::Image destination,
                                const vk::Extent2D srcSize,
                                const vk::Extent2D dstSize) {

      const auto blitRegion = vk::ImageBlit2{
          .srcSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor, .layerCount = 1},
          .srcOffsets = std::array{vk::Offset3D{},
                                   vk::Offset3D{.x = static_cast<int>(srcSize.width),
                                                .y = static_cast<int>(srcSize.height),
                                                .z = 1}},
          .dstSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor, .layerCount = 1},
          .dstOffsets = std::array{vk::Offset3D{},
                                   vk::Offset3D{.x = static_cast<int>(dstSize.width),
                                                .y = static_cast<int>(dstSize.height),
                                                .z = 1}},
      };

      const auto blitInfo = vk::BlitImageInfo2{
          .srcImage = source,
          .srcImageLayout = vk::ImageLayout::eTransferSrcOptimal,
          .dstImage = destination,
          .dstImageLayout = vk::ImageLayout::eTransferDstOptimal,
          .regionCount = 1,
          .pRegions = &blitRegion,
          .filter = vk::Filter::eLinear,
      };

      cmd.blitImage2(blitInfo);
   }
}