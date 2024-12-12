#include "DefaultRenderScheduler.hpp"
#include "CommandBufferManager.hpp"

#include <gfx/QueueTypes.hpp>

namespace tr::gfx {
   DefaultRenderScheduler::DefaultRenderScheduler(
       std::shared_ptr<task::IFrameManager> newFrameManager,
       std::shared_ptr<CommandBufferManager> newCommandBufferManager,
       std::shared_ptr<queue::Graphics> newGraphicsQueue,
       std::shared_ptr<VkResourceManager> newResourceManager)
       : frameManager{std::move(newFrameManager)},
         commandBufferManager{std::move(newCommandBufferManager)},
         graphicsQueue{std::move(newGraphicsQueue)},
         resourceManager{std::move(newResourceManager)} {

      commandBufferManager->registerType(CommandBufferType::StaticTasks);
   }

   DefaultRenderScheduler::~DefaultRenderScheduler() {
      Log.trace("Destroying DefaultRenderScheduler");
   }

   auto DefaultRenderScheduler::executeStaticTasks(Frame& frame) const -> void {
      auto& commandBuffer = frame.getCommandBuffer(CmdBufferType::Static);
      // Start Rendering and all that with command buffer

      for (const auto& task : staticRenderTasks) {
         task->record(commandBuffer);
      }

      // End Rendering and all that with command buffer
   }

   auto DefaultRenderScheduler::addStaticTask(const std::shared_ptr<task::IRenderTask> task)
       -> void {
      staticRenderTasks.push_back(task);
   }

   auto DefaultRenderScheduler::recordRenderTasks(Frame& frame) const -> void {

      const auto& startCmd = frame.getCommandBuffer(CmdBufferType::Start);
      startCmd.begin(
          vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse});

      transitionImage(startCmd,
                      resourceManager->getImage(frame.getDrawImageId()),
                      vk::ImageLayout::eUndefined,
                      vk::ImageLayout::eColorAttachmentOptimal);

      startCmd.end();

      // prepare command buffer(s) used by static tasks.

      executeStaticTasks(frame);

      // finish command buffers used by static tasks

      // Prepare command buffers used by other tasks

      // executeOtherTasks(frame);

      // finish command buffers used by other tasks

      const auto& endCmd = frame.getCommandBuffer(CmdBufferType::End);

      endCmd.begin(
          vk::CommandBufferBeginInfo{.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse});

      transitionImage(endCmd,
                      resourceManager->getImage(frame.getDrawImageId()),
                      vk::ImageLayout::eColorAttachmentOptimal,
                      vk::ImageLayout::eTransferSrcOptimal);

      transitionImage(endCmd,
                      swapchain->getSwapchainImage(frame.getSwapchainImageIndex()),
                      vk::ImageLayout::eUndefined,
                      vk::ImageLayout::eTransferDstOptimal);

      copyImageToImage(endCmd,
                       resourceManager->getImage(frame.getDrawImageId()),
                       swapchain->getSwapchainImage(frame.getSwapchainImageIndex()),
                       drawImageExtent,
                       swapchain->getImageExtent());

      transitionImage(endCmd,
                      swapchain->getSwapchainImage(frame.getSwapchainImageIndex()),
                      vk::ImageLayout::eTransferDstOptimal,
                      vk::ImageLayout::eColorAttachmentOptimal);

      endCmd.end();
   }

   auto DefaultRenderScheduler::setupCommandBuffersForFrame(Frame& frame) -> void {
      auto staticCommandBuffer =
          commandBufferManager->getCommandBuffer(frame.getIndex(), CommandBufferType::StaticTasks);
      frame.clearCommandBuffers();
      frame.addCommandBuffer(CmdBufferType::Static, std::move(staticCommandBuffer));
   }

   auto DefaultRenderScheduler::endFrame(Frame& frame) const -> void {
      // Get all the buffers one at a time because order matters
      const auto buffers = std::array{*frame.getCommandBuffer(CmdBufferType::Static)};
      const auto submitInfo = vk::SubmitInfo{
          .commandBufferCount = static_cast<uint32_t>(buffers.size()),
          .pCommandBuffers = buffers.data(),
      };
      graphicsQueue->getQueue().submit(submitInfo, frame.getInFlightFence());
   }

   auto DefaultRenderScheduler::transitionImage(const vk::raii::CommandBuffer& cmd,
                                                const vk::Image& image,
                                                vk::ImageLayout currentLayout,
                                                vk::ImageLayout newLayout) -> void {
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
