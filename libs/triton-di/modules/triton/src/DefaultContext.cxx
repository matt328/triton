#include "DefaultContext.hpp"
#include "tr/IWindow.hpp"

namespace tr {

   DefaultContext::DefaultContext(std::shared_ptr<IEventBus> newEventBus)
       : eventBus{std::move(newEventBus)} {
      std::cout << "Constructing Default Context" << '\n';
   }

   void DefaultContext::setWindow(std::shared_ptr<IWindow> newWindow) {
      std::cout << "DefaultContext::setWindow" << '\n';
      window = std::move(newWindow);
      window->registerEventBus(eventBus);
   }

   void DefaultContext::run() {
      std::cout << "DefaultContext:run" << '\n';
   }
}
