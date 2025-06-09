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

auto RenderPassFactory::createRenderPass(RenderPassType passType, PassId passId)
    -> std::unique_ptr<IRenderPass> {
  Log.trace("RenderPassFactory::createGraphicsPass()");
  switch (passType) {
    case RenderPassType::Forward:
      return std::make_unique<ForwardGraphicsPass>(imageManager,
                                                   drawContextFactory,
                                                   aliasRegistry,
                                                   pipelineFactory,
                                                   ImageUse{},
                                                   passId);
      break;
    case RenderPassType::Composition:
      Log.warn("TODO: Implement CompositionGraphicsPass");
      return nullptr;
      break;
    case RenderPassType::Culling:
      return std::make_unique<CullingPass>(drawContextFactory, pipelineFactory, passId);
      break;
    case RenderPassType::Count:
      assert(false);
      Log.warn("Should not have requested graphics pass type 'Count'");
      return nullptr;
      break;
  }
}

}
