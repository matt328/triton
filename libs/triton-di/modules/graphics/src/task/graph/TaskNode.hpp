#pragma once

namespace tr::gfx::task {
   class IRenderTask;
}

namespace tr::gfx::task::graph {

   struct TaskNode {
      std::string name;
      IRenderTask* task;
      std::vector<TaskNode*> dependencies;
   };

}
