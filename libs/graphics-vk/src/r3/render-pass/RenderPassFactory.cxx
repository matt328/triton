#include "RenderPassFactory.hpp"
#include "gfx/IFrameManager.hpp"
#include "img/ImageManager.hpp"
#include "r3/render-pass/CullingPass.hpp"
#include "r3/render-pass/PipelineFactory.hpp"
#include "ForwardGraphicsPass.hpp"

namespace tr {

RenderPassFactory::RenderPassFactory(std::shared_ptr<PipelineFactory> newPipelineFactory,
                                     std::shared_ptr<ImageManager> newImageManager,
                                     std::shared_ptr<IFrameManager> newFrameManager,
                                     std::shared_ptr<ContextFactory> newDrawContextFactory,
                                     std::shared_ptr<ResourceAliasRegistry> newAliasRegistry)
    : pipelineFactory{std::move(newPipelineFactory)},
      imageManager{std::move(newImageManager)},
      frameManager{std::move(newFrameManager)},
      drawContextFactory{std::move(newDrawContextFactory)},
      aliasRegistry{std::move(newAliasRegistry)} {
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
    return nullptr;
  };

  return std::visit(visitor, createInfo.passInfo);
}

auto RenderPassFactory::createForwardPass(PassId passId, ForwardPassCreateInfo createInfo)
    -> std::unique_ptr<IRenderPass> {
  return std::make_unique<ForwardGraphicsPass>(imageManager,
                                               drawContextFactory,
                                               aliasRegistry,
                                               pipelineFactory,
                                               createInfo,
                                               passId);
}

auto RenderPassFactory::createCullingPass(PassId passId, CullingPassCreateInfo createInfo)
    -> std::unique_ptr<IRenderPass> {
  return std::make_unique<CullingPass>(drawContextFactory, pipelineFactory, passId);
}

auto RenderPassFactory::createCompositionPass(PassId passId, CompositionPassCreateInfo createInfo)
    -> std::unique_ptr<IRenderPass> {
  // return std::make_unique<CompositionPass>(drawContextFactory, pipelineFactory, passId);
  Log.warn("TODO: implement CompositionPass");
  return nullptr;
}

}
