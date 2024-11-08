#pragma once

#include "IContext.hpp"
#include "IEventBus.hpp"

namespace tr {

   class IWindow;

   class DefaultContext : public IContext {
    public:
      explicit DefaultContext(std::shared_ptr<IEventBus> newEventBus);
      void setWindow(std::shared_ptr<IWindow> newWindow) override;
      void run() override;

    private:
      std::shared_ptr<IEventBus> eventBus;
      std::shared_ptr<IWindow> window;
   };
}