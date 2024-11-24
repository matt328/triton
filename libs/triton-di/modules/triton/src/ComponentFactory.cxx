#include "tr/ComponentFactory.hpp"
#include "Window.hpp"
#include "cm/ImGuiAdapter.hpp"
#include "tr/IGuiAdapter.hpp"
#include "tr/IGuiSystem.hpp"
#include "cm/ImGuiSystem.hpp"
#include "gfx/IRenderContext.hpp"
#include "gfx/GeometryFactory.hpp"
#include "pipeline/IShaderCompiler.hpp"
#include "pipeline/SpirvShaderCompiler.hpp"
#include "sb/LayoutFactory.hpp"
#include "sb/ShaderBindingFactory.hpp"
#include "tr/IContext.hpp"

#include "gp/action/ActionSystem.hpp"

#include "DefaultEventBus.hpp"
#include "GameplaySystem.hpp"
#include "VkGraphicsDevice.hpp"
#include "DefaultRenderContext.hpp"
#include "DefaultContext.hpp"

#include <di.hpp>

namespace di = boost::di;

namespace tr {
   auto ComponentFactory::getContext(const FrameworkConfig& config) -> std::shared_ptr<IContext> {

      constexpr auto rendererConfig = gfx::RenderContextConfig{
          .useDescriptorBuffers = false,
          .maxTextures = 16,
      };

      const auto deviceConfig =
          gfx::VkGraphicsDevice::Config{.validationLayers = {"VK_LAYER_KHRONOS_validation"},
                                        .validationEnabled = true};

      const auto injector = di::make_injector(
          di::bind<gfx::RenderContextConfig>.to(rendererConfig),
          di::bind<gfx::VkGraphicsDevice::Config>.to(deviceConfig),
          di::bind<gp::IGameplaySystem>.to<gp::GameplaySystem>(),
          di::bind<gfx::IGraphicsDevice>.to<gfx::VkGraphicsDevice>(),
          di::bind<IGuiSystem>.to<cm::ImGuiSystem>(),
          di::bind<IGuiAdapter>.to<cm::ImGuiAdapter>(),
          di::bind<tr::IWindow>.to<gfx::Window>(),
          di::bind<IEventBus>.to<DefaultEventBus>(),
          di::bind<gfx::IRenderContext>.to<gfx::DefaultRenderContext>(),
          di::bind<gfx::sb::ILayoutFactory>.to<gfx::sb::LayoutFactory>(),
          di::bind<gfx::sb::IShaderBindingFactory>.to<gfx::sb::ShaderBindingFactory>(),
          di::bind<gp::IActionSystem>.to<gp::ActionSystem>(),
          di::bind<gfx::pipe::IShaderCompiler>.to<gfx::pipe::SpirvShaderCompiler>(),
          di::bind<glm::ivec2>.to(config.initialWindowSize),
          di::bind<std::string>.to(config.windowTitle));

      return injector.create<std::shared_ptr<DefaultContext>>();
   }
}