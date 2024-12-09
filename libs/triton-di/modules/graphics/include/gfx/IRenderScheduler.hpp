#pragma once
#include <task/Frame.hpp>
#include <task/IRenderTask.hpp>

namespace tr::gfx::task {
   class IRenderScheduler {
    public:
      IRenderScheduler() = default;
      virtual ~IRenderScheduler() = default;

      IRenderScheduler(const IRenderScheduler&) = delete;
      IRenderScheduler(IRenderScheduler&&) = delete;
      auto operator=(IRenderScheduler&&) -> IRenderScheduler& = delete;
      auto operator=(const IRenderScheduler&) -> IRenderScheduler& = delete;

      virtual auto recordRenderTasks(Frame& frame) const -> void = 0;

      virtual auto executeStaticTasks(Frame& frame) const -> void = 0;
      virtual auto addStaticTask(std::shared_ptr<IRenderTask> task) -> void = 0;
   };
}
