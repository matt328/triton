#include "DefaultRenderScheduler.hpp"

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

   auto DefaultRenderScheduler::executeStaticTasks(Frame& frame) const -> void {
      const auto commandBuffer =
          commandBufferManager->getCommandBuffer(frame.getIndex(), CommandBufferType::StaticTasks);

      // Start Rendering and all that with command buffer

      for (const auto& task : staticRenderTasks) {
         task->record(*commandBuffer);
      }

      // End Rendering and all that with command buffer
   }

   auto DefaultRenderScheduler::addStaticTask(const std::shared_ptr<task::IRenderTask> task)
       -> void {
      staticRenderTasks.push_back(task);
   }

   auto DefaultRenderScheduler::recordRenderTasks(Frame& frame) const -> void {
      executeStaticTasks(frame);
   }
}
