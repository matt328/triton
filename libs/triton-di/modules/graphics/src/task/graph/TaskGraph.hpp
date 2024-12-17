#pragma once

#include "TaskNode.hpp"

namespace tr::gfx::task::graph {
   class TaskGraph {
    public:
      TaskGraph() = default;
      ~TaskGraph() = default;

      auto resolveDependencies() -> void;

    private:
      std::vector<TaskNode> taskNodes;

      auto insertBarrier(IRenderTask& producer, IRenderTask& consumer) -> void;
   };
}