#include "TaskGraph.hpp"

namespace tr::gfx::task::graph {

   // task render -> (dependency) compute

   auto TaskGraph::resolveDependencies() -> void {
      for (const auto& node : taskNodes) {
         for (auto* dep : node.dependencies) {
            insertBarrier(*dep->task, *node.task);
         }
      }
   }

   auto TaskGraph::insertBarrier(IRenderTask& producer, IRenderTask& consumer) -> void {
      /*
         - figure out what the consumer's inputs are, and see if they are among the producer's
         outputs
         - if one is found, create a Barrier for it in the consumer
      */
   }

}