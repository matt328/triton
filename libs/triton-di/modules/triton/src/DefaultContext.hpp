#pragma once

#include "tr/IContext.hpp"

namespace tr {

   class IWindow;
   class IEventBus;

   namespace gp {
      class IGameplaySystem;
   }

   namespace gfx {
      class IRenderer;
   }

   class DefaultContext : public IContext {
    public:
      explicit DefaultContext(std::shared_ptr<IEventBus> newEventBus);
      void setWindow(std::shared_ptr<IWindow> newWindow) override;
      void run() override;

    private:
      bool paused{};
      bool running{true};
      std::shared_ptr<IEventBus> eventBus;
      std::shared_ptr<IWindow> window;
      std::shared_ptr<gp::IGameplaySystem> gameplaySystem;
      std::shared_ptr<gfx::IRenderer> renderer;
   };
}