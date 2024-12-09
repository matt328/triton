#include "Frame.hpp"

#include "task/SyncManager.hpp"
#include <gfx/QueueTypes.hpp>

namespace tr::gfx {

   Frame::Frame(const uint8_t newIndex,
                std::shared_ptr<CommandBufferManager> newCommandBufferManager,
                std::shared_ptr<VkResourceManager> newResourceManager,
                std::shared_ptr<queue::Graphics> newGraphicsQueue,
                std::shared_ptr<task::SyncManager> newSyncManager)
       : index{newIndex},
         commandBufferManager{std::move(newCommandBufferManager)},
         resourceManager{std::move(newResourceManager)},
         graphicsQueue{std::move(newGraphicsQueue)},
         syncManager{std::move(newSyncManager)},
         fence{syncManager->acquireFence()} {
      drawImageName = "Frame" + std::to_string(index);
      resourceManager->createImageAndView(drawImageName, vk::Extent2D{.width = 1, .height = 1});
   }

   auto Frame::beginFrame() -> void {

      syncManager->awaitFence(fence);

      // Since the pointers refcounts should go to zero, they should be recycled.
      staticCommandBuffer =
          commandBufferManager->getCommandBuffer(index, CommandBufferType::StaticTasks);
      startBuffer = commandBufferManager->getCommandBuffer(index, CommandBufferType::StaticTasks);
      endBuffer = commandBufferManager->getCommandBuffer(index, CommandBufferType::StaticTasks);

      startBuffer->begin(vk::CommandBufferBeginInfo{});
      transitionImage(*startBuffer,
                      resourceManager->getImage(drawImageName),
                      vk::ImageLayout::eUndefined,
                      vk::ImageLayout::eColorAttachmentOptimal);
      startBuffer->end();
   }

   auto Frame::endFrame() const -> void {
      const auto buffers = std::array{(**startBuffer), **staticCommandBuffer, **endBuffer};
      const auto submitInfo = vk::SubmitInfo{
          .commandBufferCount = buffers.size(),
          .pCommandBuffers = buffers.data(),
      };
      graphicsQueue->getQueue().submit(submitInfo, fence);
   }

   auto Frame::getIndex() const -> uint8_t {
      return index;
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
