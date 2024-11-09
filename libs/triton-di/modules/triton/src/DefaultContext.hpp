#pragma once

#include "gfx/IGraphicsDevice.hpp"
#include "gp/IGameplaySystem.hpp"
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
      explicit DefaultContext(std::shared_ptr<IEventBus> newEventBus,
                              std::shared_ptr<gp::IGameplaySystem> newGameplaySystem,
                              std::shared_ptr<gfx::IRenderer> newRenderer,
                              std::shared_ptr<gfx::IGraphicsDevice> graphicsDevice,
                              std::shared_ptr<tr::IWindow> newWindow);
      void run() override;

    private:
      bool paused{};
      bool running{true};
      std::shared_ptr<IEventBus> eventBus;
      std::shared_ptr<gp::IGameplaySystem> gameplaySystem;
      std::shared_ptr<gfx::IRenderer> renderer;
      std::shared_ptr<gfx::IGraphicsDevice> graphicsDevice;
      std::shared_ptr<IWindow> window;
   };
}