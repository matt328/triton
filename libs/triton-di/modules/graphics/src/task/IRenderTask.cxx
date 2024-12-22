#include "IRenderTask.hpp"

namespace tr::gfx::task {

   [[nodiscard]] auto IRenderTask::getBarriers() const -> std::vector<graph::Barrier> {
      return pendingBarriers;
   }

   auto IRenderTask::addBarrier(graph::Barrier barrier) -> void {
      pendingBarriers.push_back(barrier);
   }

   auto getResources() -> std::vector<graph::Resource>& {
      return resources;
   }

}

