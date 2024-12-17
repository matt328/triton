#pragma once

#include "task/graph/Barrier.hpp"

namespace tr::gfx::task {
   class IRenderTask {
    public:
      IRenderTask() = default;
      virtual ~IRenderTask() = default;

      IRenderTask(const IRenderTask&) = delete;
      IRenderTask(IRenderTask&&) = delete;
      auto operator=(const IRenderTask&) -> IRenderTask& = delete;
      auto operator=(IRenderTask&&) -> IRenderTask& = delete;

      virtual auto record(vk::raii::CommandBuffer& commandBuffer) -> void = 0;
      [[nodiscard]] auto getBarriers() const -> std::vector<graph::Barrier>;

      auto addBarrier(graph::Barrier barrier) -> void;

    private:
      std::vector<graph::Barrier> pendingBarriers;
   };
}
