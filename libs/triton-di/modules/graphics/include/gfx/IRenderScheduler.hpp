#pragma once
#include "IRenderTask.hpp"

namespace tr::gfx::task {
   class IRenderScheduler {
    public:
      IRenderScheduler() = default;
      virtual ~IRenderScheduler() = default;

      IRenderScheduler(const IRenderScheduler&) = delete;
      IRenderScheduler(IRenderScheduler&&) = delete;
      auto operator=(IRenderScheduler&&) -> IRenderScheduler& = delete;
      auto operator=(const IRenderScheduler&) -> IRenderScheduler& = delete;

      virtual auto execute(vk::raii::CommandBuffer& commandBuffer) const -> void = 0;
      virtual auto addTask(std::shared_ptr<IRenderTask> task) -> void = 0;
   };
}
