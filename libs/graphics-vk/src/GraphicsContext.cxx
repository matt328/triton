#include "gfx/GraphicsContext.hpp"
#include "DefaultDebugManager.hpp"
#include "ImmediateTransferContext.hpp"
#include "VkGraphicsCreateInfo.hpp"
#include "api/fx/IEventQueue.hpp"
#include "mem/Allocator.hpp"
#include "pipeline/SpirvShaderModuleFactory.hpp"
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
#include "r3/graph/DebugFrameGraph.hpp"
#include "r3/render-pass/PipelineFactory.hpp"
#include "r3/draw-context/ContextFactory.hpp"
#include "r3/draw-context/DispatchContext.hpp"
#include "buffers/BufferSystem.hpp"

#include <di.hpp>

namespace di = boost::di;

namespace tr {

GraphicsContext::GraphicsContext(std::shared_ptr<IEventQueue> newEventQueue,
                                 std::shared_ptr<IRenderContext> newRenderContext,
                                 std::shared_ptr<IStateBuffer> newStateBuffer)
    : eventQueue{std::move(newEventQueue)},
      renderContext{std::move(newRenderContext)},
      stateBuffer{std::move(newStateBuffer)} {
}

auto GraphicsContext::create(std::shared_ptr<IEventQueue> newEventQueue,
                             std::shared_ptr<IStateBuffer> newStateBuffer,
                             std::shared_ptr<IWindow> newWindow)
    -> std::shared_ptr<GraphicsContext> {
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
                        di::bind<ImmediateTransferContext>.to<ImmediateTransferContext>(),
                        di::bind<queue::Graphics>.to<queue::Graphics>(),
                        di::bind<queue::Transfer>.to<queue::Transfer>(),
                        di::bind<queue::Present>.to<queue::Present>(),
                        di::bind<queue::Compute>.to<queue::Compute>(),
                        di::bind<CommandBufferManager>.to<CommandBufferManager>(),
                        di::bind<IShaderModuleFactory>.to<SpirvShaderModuleFactory>(),
                        di::bind<DSLayoutManager>.to<DSLayoutManager>(),
                        di::bind<IShaderBindingFactory>.to<DSShaderBindingFactory>(),
                        di::bind<Allocator>.to<Allocator>(),
                        di::bind<VkResourceManager>.to<VkResourceManager>(),
                        di::bind<IFrameManager>.to<DefaultFrameManager>(),
                        di::bind<IRenderContext>.to<R3Renderer>(),
                        di::bind<IFrameGraph>.to<DebugFrameGraph>(),
                        di::bind<PipelineFactory>.to<PipelineFactory>(),
                        di::bind<ImageManager>.to<ImageManager>(),
                        di::bind<RenderPassFactory>.to<RenderPassFactory>(),
                        di::bind<ImageRegistry>.to<ImageRegistry>());

  return injector.create<std::shared_ptr<GraphicsContext>>();
}

auto GraphicsContext::run() -> void {
  using Clock = std::chrono::steady_clock;
  auto currentTime = Clock::now();
  auto dt = std::chrono::milliseconds(16);
  auto accumulator = Clock::duration::zero();

  while (running) {
    auto newTime = Clock::now();
    auto frameTime = newTime - currentTime;
    if (frameTime > std::chrono::milliseconds(250)) {
      frameTime = std::chrono::milliseconds(250);
    }
    currentTime = newTime;
    accumulator += frameTime;

    float alpha = float(accumulator.count()) / dt.count();

    SimState prev{1024};
    SimState next{1024};

    if (stateBuffer->getInterpolatedStates(prev, next, alpha, currentTime)) {
      renderContext->setStates(prev, next, alpha);
    }

    eventQueue->dispatchPending();

    renderContext->update();
    renderContext->renderNextFrame();

    FrameMark;
  }
}

auto GraphicsContext::stop() -> void {
  running = false;
}

}
