#include "RenderPassFactory.hpp"
#include "gfx/IFrameManager.hpp"
#include "img/ImageManager.hpp"
#include "r3/render-pass//passes/CullingPass.hpp"
#include "r3/render-pass/PipelineFactory.hpp"
#include "r3/render-pass/passes/CompositionPass.hpp"
#include "r3/render-pass/passes/ForwardGraphicsPass.hpp"
#include "r3/render-pass/passes/ImGuiPass.hpp"

namespace tr {

RenderPassFactory::RenderPassFactory(std::shared_ptr<PipelineFactory> newPipelineFactory,
                                     std::shared_ptr<ImageManager> newImageManager,
                                     std::shared_ptr<IFrameManager> newFrameManager,
                                     std::shared_ptr<ContextFactory> newDrawContextFactory,
                                     std::shared_ptr<ResourceAliasRegistry> newAliasRegistry,
                                     std::shared_ptr<IShaderBindingFactory> newShaderBindingFactory,
                                     std::shared_ptr<DSLayoutManager> newLayoutManager,
                                     std::shared_ptr<Device> newDevice,
                                     std::shared_ptr<IWindow> newWindow,
                                     std::shared_ptr<Instance> newInstance,
                                     std::shared_ptr<PhysicalDevice> newPhysicalDevice,
                                     std::shared_ptr<queue::Graphics> newGraphicsQueue)
    : pipelineFactory{std::move(newPipelineFactory)},
      imageManager{std::move(newImageManager)},
      frameManager{std::move(newFrameManager)},
      drawContextFactory{std::move(newDrawContextFactory)},
      aliasRegistry{std::move(newAliasRegistry)},
      shaderBindingFactory{std::move(newShaderBindingFactory)},
      layoutManager{std::move(newLayoutManager)},
      device{std::move(newDevice)},
      window{std::move(newWindow)},
      instance{std::move(newInstance)},
      physicalDevice{std::move(newPhysicalDevice)},
      graphicsQueue{std::move(newGraphicsQueue)} {
}

auto RenderPassFactory::createRenderPass(RenderPassCreateInfo createInfo)
    -> std::unique_ptr<IRenderPass> {
  Log.trace("RenderPassFactory::createGraphicsPass()");

  const auto visitor = [&](auto&& arg) -> std::unique_ptr<IRenderPass> {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, ForwardPassCreateInfo>) {
      return createForwardPass(createInfo.passId, arg);
    }
    if constexpr (std::is_same_v<T, CullingPassCreateInfo>) {
      return createCullingPass(createInfo.passId, arg);
    }
    if constexpr (std::is_same_v<T, CompositionPassCreateInfo>) {
      return createCompositionPass(createInfo.passId, arg);
    }
    if constexpr (std::is_same_v<T, ImGuiPassCreateInfo>) {
      return createImGuiPass(createInfo.passId, arg);
    }
    return nullptr;
  };

  return std::visit(visitor, createInfo.passInfo);
}

auto RenderPassFactory::createImGuiPass(PassId passId, ImGuiPassCreateInfo createInfo)
    -> std::unique_ptr<IRenderPass> {
  return std::make_unique<ImGuiPass>(imageManager,
                                     drawContextFactory,
                                     aliasRegistry,
                                     pipelineFactory,
                                     device,
                                     window,
                                     instance,
                                     physicalDevice,
                                     graphicsQueue,
                                     createInfo,
                                     passId);
}

auto RenderPassFactory::createForwardPass(PassId passId, const ForwardPassCreateInfo& createInfo)
    -> std::unique_ptr<IRenderPass> {
  return std::make_unique<ForwardGraphicsPass>(imageManager,
                                               drawContextFactory,
                                               aliasRegistry,
                                               pipelineFactory,
                                               layoutManager,
                                               createInfo,
                                               passId);
}

auto RenderPassFactory::createCullingPass(PassId passId,
                                          [[maybe_unused]] CullingPassCreateInfo createInfo)
    -> std::unique_ptr<IRenderPass> {
  return std::make_unique<CullingPass>(drawContextFactory, pipelineFactory, passId);
}

auto RenderPassFactory::createCompositionPass(PassId passId, CompositionPassCreateInfo createInfo)
    -> std::unique_ptr<IRenderPass> {
  return std::make_unique<CompositionPass>(imageManager,
                                           drawContextFactory,
                                           aliasRegistry,
                                           pipelineFactory,
                                           shaderBindingFactory,
                                           layoutManager,
                                           createInfo,
                                           passId);
}

}
