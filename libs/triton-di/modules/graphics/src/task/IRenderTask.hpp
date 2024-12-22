#pragma once

#include "task/graph/Barrier.hpp"
#include "task/graph/Resource.hpp"

namespace tr {
   class IRenderTask {
    public:
      IRenderTask() = default;
      virtual ~IRenderTask() = default;

      IRenderTask(const IRenderTask&) = delete;
      IRenderTask(IRenderTask&&) = delete;
      auto operator=(const IRenderTask&) -> IRenderTask& = delete;
      auto operator=(IRenderTask&&) -> IRenderTask& = delete;

      virtual auto record(vk::raii::CommandBuffer& commandBuffer) -> void = 0;
      [[nodiscard]] auto getBarriers() const -> std::vector<Barrier>;

      auto addBarrier(Barrier barrier) -> void;
      [[nodiscard]] auto getResources() -> std::vector<Resource>&;

    private:
      std::vector<Barrier> pendingBarriers;
      std::vector<Resource> resources;
   };
}
