#include "VkGraphicsFactory.hpp"

#include "DefaultDebugManager.hpp"
#include "VkGraphicsContext.hpp"
#include "ResourceProxyImpl.hpp"

#include "Window.hpp"
#include "mem/Allocator.hpp"
#include "pipeline/SpirvShaderModuleFactory.hpp"
#include "r3/graph/DebugFrameGraph.hpp"
#include "r3/R3Renderer.hpp"
#include "r3/render-pass/PipelineFactory.hpp"
#include "r3/render-pass/RenderPassFactory.hpp"
#include "task/DefaultFrameManager.hpp"
#include "task/DefaultRenderScheduler.hpp"
#include "gfx/QueueTypes.hpp"

#include "task/debugshapes/DebugPipeline.hpp"
#include "vk/BufferManager.hpp"
#include "vk/CommandBufferManager.hpp"

#include "vk/sb/DSLayoutManager.hpp"
#include "vk/sb/DSShaderBindingFactory.hpp"

#include "vk/core/Context.hpp"
#include "vk/core/Surface.hpp"
#include "vk/core/Device.hpp"
#include "vk/core/PhysicalDevice.hpp"
#include "vk/core/Instance.hpp"

#include "ImmediateTransferContext.hpp"
#include "VkResourceManager.hpp"
#include "ResourceProxyImpl.hpp"
#include "VkGraphicsContext.hpp"
#include "Window.hpp"
#include "api/gfx/ImGuiSystem.hpp"

#include "mem/GeometryBuffer.hpp"
#include "mem/BufferWrapper.hpp"
#include "img/ImageRegistry.hpp"
#include "img/ImageManager.hpp"

#include <di.hpp>

namespace di = boost::di;

namespace tr {
auto createVkGraphicsContext(VkGraphicsCreateInfo createInfo,
                             std::shared_ptr<IGuiCallbackRegistrar> newGuiCallbackRegistrar,
                             std::shared_ptr<IEventBus> newEventBus,
                             std::shared_ptr<TaskQueue> newTaskQueue,
                             std::shared_ptr<IGuiAdapter> newGuiAdapter)
    -> std::shared_ptr<IGraphicsContext> {

  const auto rendererConfig = RenderContextConfig{.useDescriptorBuffers = false,
                                                  .maxStaticObjects = 1024,
                                                  .maxDynamicObjects = 1024,
                                                  .maxTerrainChunks = 1024,
                                                  .maxTextures = 16,
                                                  .framesInFlight = 2,
                                                  .initialWidth = createInfo.initialWindowSize.x,
                                                  .initialHeight = createInfo.initialWindowSize.y,
                                                  .maxDebugObjects = 32};
  const auto injector =
      di::make_injector(di::bind<RenderContextConfig>.to<>(rendererConfig),
                        di::bind<VkGraphicsCreateInfo>.to<>(createInfo),
                        di::bind<IGuiCallbackRegistrar>.to(newGuiCallbackRegistrar),
                        di::bind<IEventBus>.to<>(newEventBus),
                        di::bind<IResourceProxy>.to<ResourceProxyImpl>(),
                        di::bind<Context>.to<Context>(),
                        di::bind<IDebugManager>.to<DefaultDebugManager>(),
                        di::bind<IWindow>.to<Window>(),
                        di::bind<IGuiAdapter>.to<>(newGuiAdapter),
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
                        di::bind<IBufferManager>.to<BufferManager>(),
                        di::bind<TaskQueue>.to<>(newTaskQueue),
                        di::bind<VkResourceManager>.to<VkResourceManager>(),
                        di::bind<IFrameManager>.to<DefaultFrameManager>(),
                        di::bind<IRenderScheduler>.to<DefaultRenderScheduler>(),
                        di::bind<IRenderContext>.to<R3Renderer>(),
                        di::bind<IGuiSystem>.to<ImGuiSystem>(),
                        di::bind<IFrameGraph>.to<DebugFrameGraph>(),
                        di::bind<PipelineFactory>.to<PipelineFactory>(),
                        di::bind<ImageManager>.to<ImageManager>(),
                        di::bind<RenderPassFactory>.to<RenderPassFactory>());

  return injector.create<std::shared_ptr<VkGraphicsContext>>();
}

}
