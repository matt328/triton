#include "TaskGraph.hpp"

namespace tr::gfx::task::graph {

   // task render -> (dependency) compute

   auto TaskGraph::resolveDependencies() -> void {
      for (const auto& node : taskNodes) {
         for (auto* dep : node.dependencies) {
            insertBarrier({.producer = *dep->task, .consumer = *node.task});
         }
      }
   }

   auto TaskGraph::insertBarrier(const BarrierConfig& barrierConfig) -> void {
      /*
         - figure out what the consumer's inputs are, and see if they are among the producer's
         outputs
         - if one is found, create a Barrier for it in the consumer
      */
   }

}