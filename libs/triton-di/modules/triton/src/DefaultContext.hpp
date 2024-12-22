#pragma once

#include "gp/Registry.hpp"
#include "tr/IGuiSystem.hpp"
#include "gfx/IGraphicsDevice.hpp"
#include "gfx/IRenderContext.hpp"
#include "tr/IContext.hpp"

namespace tr {

class IWindow;
class IEventBus;
class IGameplaySystem;

class DefaultContext final : public IContext {
 public:
   explicit DefaultContext(std::shared_ptr<IEventBus> newEventBus,
                           std::shared_ptr<IRenderContext> newRenderContext,
                           std::shared_ptr<IWindow> newWindow,
                           std::shared_ptr<IGuiSystem> newGuiSystem,
                           std::shared_ptr<Registry> newRegistry);
   void run() override;
   auto getGameplaySystem() -> std::shared_ptr<IGameplaySystem> override;
   auto getGuiSystem() -> std::shared_ptr<tr::IGuiSystem> override;
   auto getEventSystem() -> std::shared_ptr<IEventBus> override;
   auto getRegistry() -> std::shared_ptr<Registry> override;

 private:
   bool paused{};
   bool running{true};
   std::shared_ptr<IEventBus> eventBus;
   std::shared_ptr<IRenderContext> renderContext;
   std::shared_ptr<IWindow> window;
   std::shared_ptr<tr::IGuiSystem> guiSystem;
   std::shared_ptr<Registry> registry;
};
}
