#include "DefaultRenderScheduler.hpp"
#include "CommandBufferManager.hpp"

/*
 * Keep Frame a mostly POD structure with a few utility functions until the per frame logic becomes
 * too much for the RenderScheduler
 * TODO(matt)
 * FrameManager should be what interacts with CommandBufferManager
 * It will need to know what types of command buffers what execute*Tasks require. That's ok, not
 * every component can live in complete isolation from the others. It's more like the render tasks
 * don't know or care about the command buffer they're using, they just know how to record their
 * commands into the given command buffer.
 */

namespace tr::gfx {
   DefaultRenderScheduler::DefaultRenderScheduler(
       std::shared_ptr<task::IFrameManager> newFrameManager,
       std::shared_ptr<CommandBufferManager> newCommandBufferManager)
       : frameManager{std::move(newFrameManager)},
         commandBufferManager{std::move(newCommandBufferManager)} {

      commandBufferManager->registerType(CommandBufferType::StaticTasks);
   }

   DefaultRenderScheduler::~DefaultRenderScheduler() {
      Log.trace("Destroying DefaultRenderScheduler");
   }

   auto DefaultRenderScheduler::executeStaticTasks(Frame& frame) const -> void {
      auto& commandBuffer = frame.getStaticCommandBuffer();
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
      frame.setStaticCommandBuffer(std::move(staticCommandBuffer));
   }
}
