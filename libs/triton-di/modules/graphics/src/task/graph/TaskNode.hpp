#pragma once

namespace tr {
   class IRenderTask;
}

namespace tr {

   struct TaskNode {
      std::string name;
      IRenderTask* task;
      std::vector<TaskNode*> dependencies;
   };

}
