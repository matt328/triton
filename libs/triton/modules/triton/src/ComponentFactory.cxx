#include "tr/ComponentFactory.hpp"

#include <di.hpp>

#include "DefaultDebugService.hpp"
#include "Window.hpp"
#include "cm/ImGuiAdapter.hpp"
#include "gp/DefaultGameplaySystem.hpp"
#include "pipeline/IndirectPipeline.hpp"
#include "tr/IGameplaySystem.hpp"
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
#include "tr/DebugTerrainSystem.hpp"
#include "vk/core/Swapchain.hpp"
#include "vk/core/Surface.hpp"
#include "gfx/QueueTypes.hpp"
#include "vk/core/Device.hpp"
#include "gfx/RenderContextConfig.hpp"

#include "task/DefaultFrameManager.hpp"
#include "task/DefaultRenderScheduler.hpp"

#include "pipeline/SpirvShaderModuleFactory.hpp"
#include "vk/sb/DSShaderBindingFactory.hpp"
#include "vk/BufferManager.hpp"
#include "cm/TaskQueue.hpp"
#include "tr/ITerrainSystem.hpp"

// These classes are just implicitly bound. Maybe this isn't a great idea, but it does work.
#include "geo/GeometryData.hpp"
#include "vk/sb/IShaderBinding.hpp"
#include "vk/sb/DSLayout.hpp"
#include "tr/SdfGenerator.hpp"
#include "systems/CameraSystem.hpp"
#include "systems/AnimationSystem.hpp"
#include "systems/RenderDataSystem.hpp"
#include "systems/TransformSystem.hpp"

namespace di = boost::di;

namespace tr {

auto ComponentFactory::getContext(const FrameworkConfig& config) -> std::shared_ptr<IContext> {

  constexpr auto rendererConfig = RenderContextConfig{.useDescriptorBuffers = false,
                                                      .maxStaticObjects = 1024,
                                                      .maxDynamicObjects = 1024,
                                                      .maxTerrainChunks = 27,
                                                      .maxTextures = 16,
                                                      .framesInFlight = 2};

  constexpr auto taskQueueConfig = tr::TaskQueueConfig{.maxQueueSize = 100};

  // TODO(matt): Make it so each 'module' configures and produces its own injector, similar to how
  // this one is exposed to the application.
  /*
    Each module needs to expose a single top level interface that
  */

  const auto injector =
      di::make_injector(di::bind<IDebugManager>.to<DefaultDebugManager>(),
                        di::bind<RenderContextConfig>.to(rendererConfig),
                        di::bind<IGuiSystem>.to<ImGuiSystem>(),
                        di::bind<IGuiAdapter>.to<ImGuiAdapter>(),
                        di::bind<IWindow>.to<Window>(),
                        di::bind<IEventBus>.to<DefaultEventBus>(),
                        di::bind<IRenderContext>.to<NewRenderContext>(),
                        di::bind<IRenderScheduler>.to<DefaultRenderScheduler>(),
                        di::bind<IFrameManager>.to<DefaultFrameManager>(),
                        di::bind<IActionSystem>.to<ActionSystem>(),
                        di::bind<IGameplaySystem>.to<DefaultGameplaySystem>(),
                        di::bind<glm::ivec2>.to(config.initialWindowSize),
                        di::bind<std::string>.to(config.windowTitle),
                        di::bind<Device>.to<Device>(),
                        di::bind<Surface>.to<Surface>(),
                        di::bind<Swapchain>.to<Swapchain>(),
                        di::bind<IShaderModuleFactory>.to<SpirvShaderModuleFactory>(),
                        di::bind<IShaderBindingFactory>.to<DSShaderBindingFactory>(),
                        di::bind<queue::Graphics>.to<queue::Graphics>(),
                        di::bind<queue::Transfer>.to<queue::Transfer>(),
                        di::bind<queue::Present>.to<queue::Present>(),
                        di::bind<queue::Compute>.to<queue::Compute>(),
                        di::bind<IBufferManager>.to<BufferManager>(),
                        di::bind<tr::TaskQueueConfig>.to(taskQueueConfig),
                        di::bind<tr::ITerrainSystem>.to<tr::DebugTerrainSystem>(),
                        di::bind<tr::IDebugService>.to<tr::DefaultDebugService>());

  return injector.create<std::shared_ptr<DefaultContext>>();
}
}
