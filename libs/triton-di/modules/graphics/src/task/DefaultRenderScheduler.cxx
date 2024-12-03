#include "DefaultRenderScheduler.hpp"

namespace tr::gfx {
   auto DefaultRenderScheduler::execute(vk::raii::CommandBuffer& commandBuffer) const -> void {
      for (const auto& task : tasks) {
         task->record(commandBuffer);
      }
   }

   auto DefaultRenderScheduler::addTask(const std::shared_ptr<task::IRenderTask> task) -> void {
      tasks.push_back(task);
   }
}
