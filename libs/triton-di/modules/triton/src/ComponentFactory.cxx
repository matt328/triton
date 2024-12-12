#include "tr/ComponentFactory.hpp"
#include "Window.hpp"
#include "cm/ImGuiAdapter.hpp"
#include "tr/IGuiAdapter.hpp"
#include "tr/IGuiSystem.hpp"
#include "cm/ImGuiSystem.hpp"
#include "gfx/IRenderContext.hpp"
#include "tr/IContext.hpp"

#include "gp/action/ActionSystem.hpp"

#include "DefaultEventBus.hpp"
#include "DefaultContext.hpp"
#include "DefaultDebugManager.hpp"
#include "NewRenderContext.hpp"
#include "vk/Swapchain.hpp"
#include "vk/Surface.hpp"
#include "gfx/QueueTypes.hpp"
#include "vk/Device.hpp"
#include "gfx/RenderContextConfig.hpp"

#include <di.hpp>
#include <task/DefaultFrameManager.hpp>
#include <task/DefaultRenderScheduler.hpp>

namespace di = boost::di;

namespace tr {
   auto ComponentFactory::getContext(const FrameworkConfig& config) -> std::shared_ptr<IContext> {

      constexpr auto rendererConfig = gfx::RenderContextConfig{.useDescriptorBuffers = false,
                                                               .maxTextures = 16,
                                                               .framesInFlight = 2};

      const auto injector =
          di::make_injector(di::bind<gfx::IDebugManager>.to<gfx::DefaultDebugManager>(),
                            di::bind<gfx::RenderContextConfig>.to(rendererConfig),
                            di::bind<IGuiSystem>.to<cm::ImGuiSystem>(),
                            di::bind<IGuiAdapter>.to<cm::ImGuiAdapter>(),
                            di::bind<IWindow>.to<gfx::Window>(),
                            di::bind<IEventBus>.to<DefaultEventBus>(),
                            di::bind<gfx::IRenderContext>.to<gfx::NewRenderContext>(),
                            di::bind<gfx::task::IRenderScheduler>.to<gfx::DefaultRenderScheduler>(),
                            di::bind<gfx::task::IFrameManager>.to<gfx::task::DefaultFrameManager>(),
                            di::bind<gp::IActionSystem>.to<gp::ActionSystem>(),
                            di::bind<glm::ivec2>.to(config.initialWindowSize),
                            di::bind<std::string>.to(config.windowTitle),
                            di::bind<gfx::Device>.to<gfx::Device>(),
                            di::bind<gfx::queue::Graphics>.to<gfx::queue::Graphics>(),
                            di::bind<gfx::queue::Transfer>.to<gfx::queue::Transfer>(),
                            di::bind<gfx::queue::Present>.to<gfx::queue::Present>(),
                            di::bind<gfx::queue::Compute>.to<gfx::queue::Compute>());

      return injector.create<std::shared_ptr<DefaultContext>>();
   }
}
