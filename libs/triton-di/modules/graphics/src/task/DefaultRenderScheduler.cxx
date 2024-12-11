#include "DefaultRenderScheduler.hpp"
#include "CommandBufferManager.hpp"

#include <gfx/QueueTypes.hpp>

namespace tr::gfx {
   DefaultRenderScheduler::DefaultRenderScheduler(
       std::shared_ptr<task::IFrameManager> newFrameManager,
       std::shared_ptr<CommandBufferManager> newCommandBufferManager,
       std::shared_ptr<queue::Graphics> newGraphicsQueue)
       : frameManager{std::move(newFrameManager)},
         commandBufferManager{std::move(newCommandBufferManager)},
         graphicsQueue{std::move(newGraphicsQueue)} {

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

      // Record image transitions to start command buffer

      // prepare command buffer(s) used by static tasks.

      executeStaticTasks(frame);

      // finish command buffers used by static tasks

      // Prepare command buffers used by other tasks

      // executeOtherTasks(frame);

      // finish command buffers used by other tasks

      // Record image transitions to the end command buffer
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
}
