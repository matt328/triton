#pragma once

#include "gfx/IGraphicsDevice.hpp"
#include "gfx/IRenderContext.hpp"
#include "tr/IContext.hpp"

namespace tr {

   class IWindow;
   class IEventBus;

   namespace gp {
      class IGameplaySystem;
   }

   class DefaultContext : public IContext {
    public:
      explicit DefaultContext(std::shared_ptr<IEventBus> newEventBus,
                              std::shared_ptr<gp::IGameplaySystem> newGameplaySystem,
                              std::shared_ptr<gfx::IRenderContext> newRenderContext,
                              std::shared_ptr<gfx::IGraphicsDevice> graphicsDevice,
                              std::shared_ptr<tr::IWindow> newWindow);
      void run() override;
      auto getGameplaySystem() -> std::shared_ptr<gp::IGameplaySystem> override;

    private:
      bool paused{};
      bool running{true};
      std::shared_ptr<IEventBus> eventBus;
      std::shared_ptr<gp::IGameplaySystem> gameplaySystem;
      std::shared_ptr<gfx::IRenderContext> renderContext;
      std::shared_ptr<gfx::IGraphicsDevice> graphicsDevice;
      std::shared_ptr<IWindow> window;
   };
}