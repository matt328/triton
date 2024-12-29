#pragma once

#include "TaskNode.hpp"

/*
  Tasks will define ResourceUsages
  - ResourceUsage will contain everything needed to create a barrier to ensure the incoming resource
  is ready to be used. I *think* that is all that is needed, just to create barriers.
  - ResourceUsage will also either own or map to a resource 'handle'. The RenderScheduler will
  assign resource handles to slots in each task, this is how a task will know which buffers and
  images to use. tasks will be dumb and just operate on whatever resource handles they're given.
  - This is all completely independent of the TaskGraph.
  - Maybe code this up, and orchestrate it manually in a ManualRenderScheduler to solidify the API.


   CullingTask
   - input: ObjectBoundsBuffer
   - output: IndirectCommandBuffer
   - output: DrawCountBuffer

   StaticRenderTask
   - input DrawCountBuffer
   - input: IndirectCommandBuffer
   - output: drawImage
*/

namespace tr {

struct BarrierConfig {
  IRenderTask* producer;
  IRenderTask* consumer;
};

class TaskGraph {
public:
  TaskGraph() = default;
  ~TaskGraph() = default;

  TaskGraph(TaskGraph&& taskGraph) = delete;
  TaskGraph(const TaskGraph& taskGraph) = delete;
  auto operator=(TaskGraph&& taskGraph) = delete;
  auto operator=(const TaskGraph& taskGraph) = delete;

  auto resolveDependencies() -> void;

private:
  std::vector<TaskNode> taskNodes;

  /// Inserts a Barrier here.
  auto insertBarrier(const BarrierConfig& barrierConfig) -> void;
};

} // namespace tr
