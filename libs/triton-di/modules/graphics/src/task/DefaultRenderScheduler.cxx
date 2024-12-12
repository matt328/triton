#include "DefaultRenderScheduler.hpp"
#include "CommandBufferManager.hpp"

#include <gfx/QueueTypes.hpp>

namespace tr::gfx {
   DefaultRenderScheduler::DefaultRenderScheduler(
       std::shared_ptr<task::IFrameManager> newFrameManager,
       std::shared_ptr<CommandBufferManager> newCommandBufferManager,
       std::shared_ptr<queue::Graphics> newGraphicsQueue,
       std::shared_ptr<VkResourceManager> newResourceManager,
       std::shared_ptr<Swapchain> newSwapchain)
       : frameManager{std::move(newFrameManager)},
         commandBufferManager{std::move(newCommandBufferManager)},
         graphicsQueue{std::move(newGraphicsQueue)},
         resourceManager{std::move(newResourceManager)},
         swapchain{std::move(newSwapchain)} {

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
                       resourceManager->getImageExtent(frame.getDrawImageId()),
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
      auto startCommandBuffer =
          commandBufferManager->getCommandBuffer(frame.getIndex(), CommandBufferType::StaticTasks);
      auto endCommandBuffer =
          commandBufferManager->getCommandBuffer(frame.getIndex(), CommandBufferType::StaticTasks);
      frame.clearCommandBuffers();
      frame.addCommandBuffer(CmdBufferType::Static, std::move(staticCommandBuffer));
      frame.addCommandBuffer(CmdBufferType::Start, std::move(startCommandBuffer));
      frame.addCommandBuffer(CmdBufferType::End, std::move(endCommandBuffer));
   }

   auto DefaultRenderScheduler::endFrame(Frame& frame) const -> void {
      // Get all the buffers one at a time because order matters
      const auto buffers = std::array{*frame.getCommandBuffer(CmdBufferType::Start),
                                      //*frame.getCommandBuffer(CmdBufferType::Static),
                                      *frame.getCommandBuffer(CmdBufferType::End)};
      const auto submitInfo = vk::SubmitInfo{
          .commandBufferCount = static_cast<uint32_t>(buffers.size()),
          .pCommandBuffers = buffers.data(),
      };
      try {
         graphicsQueue->getQueue().submit(submitInfo, *frame.getInFlightFence());
      } catch (const std::exception& ex) {
         Log.error("Failed to submit command buffer submission {}", ex.what());
      }
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
   auto DefaultRenderScheduler::copyImageToImage(const vk::raii::CommandBuffer& cmd,
                                                 const vk::Image source,
                                                 const vk::Image destination,
                                                 const vk::Extent2D srcSize,
                                                 const vk::Extent2D dstSize) -> void {
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
