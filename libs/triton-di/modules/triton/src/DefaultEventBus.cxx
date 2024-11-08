#include "DefaultEventBus.hpp"

namespace tr {
   DefaultEventBus::DefaultEventBus() {
      std::cout << "Constructing EventBus" << '\n';
   }

   void DefaultEventBus::sendEvent() {
   }
}
