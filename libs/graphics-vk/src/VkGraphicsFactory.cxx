#include "VkGraphicsFactory.hpp"

#include "DefaultDebugManager.hpp"
#include "VkGraphicsContext.hpp"
#include "ResourceProxyImpl.hpp"

#include "Window.hpp"
#include "mem/Allocator.hpp"
#include "pipeline/SpirvShaderModuleFactory.hpp"
#include "vk/BufferManager.hpp"
#include "vk/CommandBufferManager.hpp"
#include "vk/core/Context.hpp"
#include "vk/core/Surface.hpp"
#include "gfx/QueueTypes.hpp"
#include "vk/sb/DSLayoutManager.hpp"
#include "vk/sb/DSShaderBindingFactory.hpp"
#include "vk/core/Device.hpp"
#include "vk/core/PhysicalDevice.hpp"
#include "ImmediateTransferContext.hpp"
#include "vk/core/Instance.hpp"
#include "VkResourceManager.hpp"
#include "ResourceProxyImpl.hpp"
#include "VkGraphicsContext.hpp"

#include <di.hpp>

namespace di = boost::di;

namespace tr {
auto createVkGraphicsContext(std::shared_ptr<IGuiCallbackRegistrar> newGuiCallbackRegistrar,
                             const std::shared_ptr<IEventBus>& newEventBus,
                             std::shared_ptr<ITaskQueue> newTaskQueue,
                             const std::shared_ptr<IGuiAdapter>& newGuiAdapter)
    -> std::shared_ptr<IGraphicsContext> {
  const auto injector =
      di::make_injector(di::bind<IGuiCallbackRegistrar>.to(newGuiCallbackRegistrar),
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
                        di::bind<ITaskQueue>.to<>(newTaskQueue),
                        di::bind<VkResourceManager>.to<VkResourceManager>());

  return injector.create<std::shared_ptr<VkGraphicsContext>>();
}

}
