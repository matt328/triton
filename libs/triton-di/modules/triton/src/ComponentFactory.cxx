#include "tr/ComponentFactory.hpp"
#include "Window.hpp"
#include "tr/IContext.hpp"

#include "gp/action/ActionSystem.hpp"

#include "DefaultEventBus.hpp"
#include "GameplaySystem.hpp"
#include "VkGraphicsDevice.hpp"
#include "Renderer.hpp"
#include "DefaultContext.hpp"

#include <di.hpp>

namespace di = boost::di;

namespace tr {
   auto ComponentFactory::getContext(const FrameworkConfig& config) -> std::shared_ptr<IContext> {

      const auto injector =
          di::make_injector(di::bind<tr::IWindow>.to<gfx::Window>(),
                            di::bind<IEventBus>.to<DefaultEventBus>(),
                            di::bind<gp::IGameplaySystem>.to<gp::GameplaySystem>(),
                            di::bind<gfx::IGraphicsDevice>.to<gfx::VkGraphicsDevice>(),
                            di::bind<gfx::IRenderer>.to<gfx::Renderer>(),
                            di::bind<gp::IActionSystem>.to<gp::ActionSystem>(),
                            di::bind<glm::ivec2>.to(config.initialWindowSize),
                            di::bind<std::string>.to(config.windowTitle));

      return injector.create<std::shared_ptr<DefaultContext>>();
   }
}