#include "DefaultRenderScheduler.hpp"

namespace tr::gfx {
   auto DefaultRenderScheduler::executeStaticTasks(vk::raii::CommandBuffer& commandBuffer) const
       -> void {
      for (const auto& task : staticRenderTasks) {
         task->record(commandBuffer);
      }
   }

   auto DefaultRenderScheduler::addStaticTask(const std::shared_ptr<task::IRenderTask> task) -> void {
      staticRenderTasks.push_back(task);
   }
}
