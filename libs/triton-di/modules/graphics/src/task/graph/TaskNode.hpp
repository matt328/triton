#pragma once

namespace tr {

class IRenderTask;

struct TaskNode {
  std::string name;
  IRenderTask* task;
  std::vector<TaskNode*> dependencies;
};

}
