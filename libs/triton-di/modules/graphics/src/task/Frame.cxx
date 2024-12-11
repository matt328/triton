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

   auto Frame::getImageAvailableSemaphore() -> vk::raii::Semaphore& {
      return imageAvailableSemaphore;
   }

   auto Frame::getInFlightFence() -> vk::raii::Fence& {
      return inFlightFence;
   }

   auto Frame::getSwapchainImageIndex() const -> uint32_t {
      return swapchainImageIndex;
   }

   auto Frame::setSwapchainImageIndex(const uint32_t index) -> void {
      swapchainImageIndex = index;
   }

   auto Frame::addCommandBuffer(CmdBufferType cmdType, CommandBufferPtr&& commandBuffer) -> void {
      commandBuffers.insert(std::make_pair(cmdType, std::move(commandBuffer)));
   }

   auto Frame::getCommandBuffer(const CmdBufferType cmdType) const -> vk::raii::CommandBuffer& {
      return *commandBuffers.at(cmdType);
   }

   auto Frame::clearCommandBuffers() -> void {
      // Only call clear command buffers if they are not being used
      assert(inFlightFence.getStatus() == vk::Result::eSuccess);
      commandBuffers.clear();
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
