#pragma once

#include "TaskNode.hpp"

/*
   - Buffers are owned by a frame
   - RenderTasks' abstraction over inputs and outputs should be 'classes' of buffers/images when
   defining the dependencies
   - When actually creating barriers, match up the buffer/image's class with the actual buffer/image
   from the current Frame
   - inputs and outputs abstraction == TaskResource
   - how should a TaskResource call out a 'class' of buffers/images.
   - should incorporate an ID or Tag somehow on the mem::buffer?
   - does defining a TaskResource cause the creation of the buffer/image?

   - Maybe on init or change, the Task can provide its TaskResources and register them with each
   frame. That way the frame can own the mem::Buffer, and hand out a handle to it to be used in the
   TaskResource and when recording the command buffer, the Task will just ask the current frame for
   the mem::Buffer at the handle supplied by the Pending Barrier
   - Calculating the dependencies would involve creating a PendingBarrier using info from the
   'overlapping' TaskResource

   CullingTask
   - input: ObjectBoundsBuffer
   - output: IndirectCommandBuffer
   - output: DrawCountBuffer

   StaticRenderTask
   - input DrawCountBuffer
   - input: IndirectCommandBuffer
   - output: drawImage
*/

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