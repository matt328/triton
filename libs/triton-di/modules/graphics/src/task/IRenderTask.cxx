#include "IRenderTask.hpp"

namespace tr {

   [[nodiscard]] auto IRendergetBarriers() const -> std::vector<Barrier> {
      return pendingBarriers;
   }

   auto IRenderaddBarrier(Barrier barrier) -> void {
      pendingBarriers.push_back(barrier);
   }

   auto getResources() -> std::vector<Resource>& {
      return resources;
   }

}

