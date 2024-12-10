#include "Frame.hpp"

namespace tr::gfx {

   Frame::Frame(const uint8_t newIndex,
                vk::raii::Fence&& newRenderFence,
                vk::raii::Semaphore&& newImageAvailableSemaphore,
                vk::raii::Semaphore&& newRenderFinishedSemaphore)
       : index{newIndex},
         inFlightFence{std::move(newRenderFence)},
         imageAvailableSemaphore{std::move(newImageAvailableSemaphore)},
         renderFinishedSemaphore{std::move(newRenderFinishedSemaphore)} {
      drawImageName = "Frame" + std::to_string(index);
   }

   auto Frame::getIndex() const -> uint8_t {
      return index;
   }

   auto Frame::getStaticCommandBuffer() const -> vk::raii::CommandBuffer& {
      return *staticCommandBuffer;
   }

   auto Frame::getImageAvailableSemaphore() -> vk::raii::Semaphore& {
      return imageAvailableSemaphore;
   }

   auto Frame::getInFlightFence() -> vk::raii::Fence& {
      return inFlightFence;
   }
   auto Frame::getSwapchainImageIndex() const -> uint32_t {
      return swapchainImageIndex;
   }

   auto Frame::setStaticCommandBuffer(CommandBufferPtr&& buffer) -> void {
      staticCommandBuffer = std::move(buffer);
   }

   auto Frame::setSwapchainImageIndex(const uint32_t index) -> void {
      swapchainImageIndex = index;
   }

   auto Frame::transitionImage(const vk::raii::CommandBuffer& cmd,
                               const vk::Image& image,
                               const vk::ImageLayout currentLayout,
                               const vk::ImageLayout newLayout) -> void {
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
}
