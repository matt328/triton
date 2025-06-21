#include "gfx/GraphicsContext.hpp"
#include "FrameState.hpp"
#include "gfx/QueueTypes.hpp"
#include "r3/GeometryBufferPack.hpp"
#include "r3/graph/OrderedFrameGraph.hpp"
#include "r3/graph/ResourceAliasRegistry.hpp"
#include "resources/DefaultAssetSystem.hpp"
#include "DefaultDebugManager.hpp"
#include "api/fx/IEventQueue.hpp"
#include "mem/Allocator.hpp"
#include "pipeline/SpirvShaderModuleFactory.hpp"
#include "resources/TransferSystem.hpp"
#include "resources/allocators/GeometryAllocator.hpp"
#include "task/DefaultFrameManager.hpp"

#include "vk/core/Instance.hpp"
#include "vk/core/Surface.hpp"
#include "vk/core/Context.hpp"
#include "vk/core/Swapchain.hpp"
#include "vk/core/Device.hpp"
#include "vk/core/PhysicalDevice.hpp"
#include "vk/sb/DSLayoutManager.hpp"
#include "vk/sb/DSShaderBindingFactory.hpp"
#include "img/ImageRegistry.hpp"
#include "img/ImageManager.hpp"
#include "r3/render-pass/RenderPassFactory.hpp"
#include "r3/R3Renderer.hpp"
#include "r3/render-pass/PipelineFactory.hpp"
#include "r3/draw-context/ContextFactory.hpp"
#include "r3/draw-context/IDispatchContext.hpp"
#include "buffers/BufferSystem.hpp"

#include "resources/allocators/GeometryDispatcher.hpp"
#include "gfx/GeometryHandleMapper.hpp"
#include "vk/command-buffer/CommandBufferManager.hpp"

#define BOOST_DI_CFG_CTOR_LIMIT_SIZE 20
#include <di.hpp>

namespace di = boost::di;

namespace tr {

constexpr uint32_t MaxFrameTime = 250;

GraphicsContext::GraphicsContext(std::shared_ptr<IEventQueue> newEventQueue,
                                 std::shared_ptr<IRenderContext> newRenderContext,
                                 std::shared_ptr<IStateBuffer> newStateBuffer,
                                 std::shared_ptr<Device> newDevice,
                                 std::shared_ptr<IAssetSystem> newAssetSystem)
    : eventQueue{std::move(newEventQueue)},
      renderContext{std::move(newRenderContext)},
      stateBuffer{std::move(newStateBuffer)},
      device{std::move(newDevice)},
      assetSystem{std::move(newAssetSystem)} {
}

GraphicsContext::~GraphicsContext() {
  Log.trace("Destroying GraphicsContext");
}

auto GraphicsContext::create(std::shared_ptr<IEventQueue> newEventQueue,
                             std::shared_ptr<IStateBuffer> newStateBuffer,
                             std::shared_ptr<IWindow> newWindow,
                             std::shared_ptr<IAssetService> newAssetService)
    -> std::shared_ptr<GraphicsContext> {
  Log.trace("GraphicsContext::create()");
  const auto rendererConfig = RenderContextConfig{.useDescriptorBuffers = false,
                                                  .maxStaticObjects = 1024,
                                                  .maxDynamicObjects = 1024,
                                                  .maxTerrainChunks = 1024,
                                                  .maxTextures = 16,
                                                  .framesInFlight = 2,
                                                  .initialWidth = 1920,
                                                  .initialHeight = 1080,
                                                  .maxDebugObjects = 32};

  const auto injector =
      di::make_injector(di::bind<IEventQueue>.to<>(newEventQueue),
                        di::bind<IStateBuffer>.to<>(newStateBuffer),
                        di::bind<IWindow>.to<>(newWindow),
                        di::bind<RenderContextConfig>.to<>(rendererConfig),
                        di::bind<Context>.to<Context>(),
                        di::bind<IDebugManager>.to<DefaultDebugManager>(),
                        di::bind<Device>.to<Device>(),
                        di::bind<PhysicalDevice>.to<PhysicalDevice>(),
                        di::bind<Surface>.to<Surface>(),
                        di::bind<Instance>.to<Instance>(),
                        di::bind<Swapchain>.to<Swapchain>(),
                        di::bind<queue::Graphics>.to<queue::Graphics>(),
                        di::bind<queue::Transfer>.to<queue::Transfer>(),
                        di::bind<queue::Present>.to<queue::Present>(),
                        di::bind<queue::Compute>.to<queue::Compute>(),
                        di::bind<CommandBufferManager>.to<CommandBufferManager>(),
                        di::bind<IShaderModuleFactory>.to<SpirvShaderModuleFactory>(),
                        di::bind<DSLayoutManager>.to<DSLayoutManager>(),
                        di::bind<IShaderBindingFactory>.to<DSShaderBindingFactory>(),
                        di::bind<Allocator>.to<Allocator>(),
                        di::bind<IFrameManager>.to<DefaultFrameManager>(),
                        di::bind<IRenderContext>.to<R3Renderer>(),
                        di::bind<IFrameGraph>.to<OrderedFrameGraph>(),
                        di::bind<PipelineFactory>.to<PipelineFactory>(),
                        di::bind<ImageManager>.to<ImageManager>(),
                        di::bind<BufferSystem>.to<BufferSystem>(),
                        di::bind<ContextFactory>.to<ContextFactory>(),
                        di::bind<RenderPassFactory>.to<RenderPassFactory>(),
                        di::bind<IAssetService>.to<>(newAssetService),
                        di::bind<GeometryBufferPack>.to<GeometryBufferPack>(),
                        di::bind<FrameState>.to<FrameState>(),
                        di::bind<GeometryHandleMapper>.to(std::make_shared<GeometryHandleMapper>()),
                        di::bind<IAssetSystem>.to<DefaultAssetSystem>(),
                        di::bind<ResourceAliasRegistry>.to<ResourceAliasRegistry>());

  return injector.create<std::shared_ptr<GraphicsContext>>();
}

// NOLINTNEXTLINE
auto GraphicsContext::run(std::stop_token token) -> void {
  using Clock = std::chrono::steady_clock;
  auto currentTime = Clock::now();

  assetSystem->run();

  while (!token.stop_requested()) {
    auto newTime = Clock::now();
    auto frameTime = newTime - currentTime;
    if (frameTime > std::chrono::milliseconds(MaxFrameTime)) {
      frameTime = std::chrono::milliseconds(MaxFrameTime);
    }
    currentTime = newTime;

    eventQueue->dispatchPending();
    renderContext->renderNextFrame();
  }
  Log.trace("graphicsThread token stop_requested()");
  device->waitIdle();
  assetSystem->requestStop();
}

}
