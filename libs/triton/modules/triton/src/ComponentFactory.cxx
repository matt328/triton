#include "tr/ComponentFactory.hpp"

#include <di.hpp>

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
#include "vk/core/Swapchain.hpp"
#include "vk/core/Surface.hpp"
#include "gfx/QueueTypes.hpp"
#include "vk/core/Device.hpp"
#include "gfx/RenderContextConfig.hpp"

#include "task/DefaultFrameManager.hpp"
#include "task/DefaultRenderScheduler.hpp"

#include "pipeline/SpirvShaderModuleFactory.hpp"
#include "vk/sb/DSShaderBindingFactory.hpp"
#include "vk/sb/IShaderBinding.hpp"
#include "vk/sb/DSLayout.hpp"
#include "vk/BufferManager.hpp"
#include "geo/GeometryData.hpp"
#include "cm/TaskQueue.hpp"

namespace di = boost::di;

namespace tr {

auto ComponentFactory::getContext(const FrameworkConfig& config) -> std::shared_ptr<IContext> {

  constexpr auto rendererConfig = RenderContextConfig{.useDescriptorBuffers = false,
                                                      .maxStaticObjects = 1024,
                                                      .maxDynamicObjects = 1024,
                                                      .maxTextures = 16,
                                                      .framesInFlight = 2};

  constexpr auto taskQueueConfig = tr::TaskQueueConfig{.maxQueueSize = 100};

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
                        di::bind<tr::TaskQueueConfig>.to(taskQueueConfig));

  return injector.create<std::shared_ptr<DefaultContext>>();
}
}
