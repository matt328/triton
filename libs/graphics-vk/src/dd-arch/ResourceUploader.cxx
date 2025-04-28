#include "dd/ResourceUploader.hpp"
#include "dd/DrawContext.hpp"
#include "dd/DrawContextFactory.hpp"
#include "dd/RenderConfigRegistry.hpp"

namespace tr {

ResourceUploader::ResourceUploader(std::shared_ptr<RenderConfigRegistry> newRenderConfigRegistry,
                                   std::shared_ptr<DrawContextFactory> newDrawContextFactory)
    : renderConfigRegistry{std::move(newRenderConfigRegistry)},
      drawContextFactory{std::move(newDrawContextFactory)} {
}

auto ResourceUploader::registerRenderable([[maybe_unused]] const RenderableData& data)
    -> RenderableResources {
  return {};
}

}
/*
  Create a RenderPass class. It is just a 'slice' of DrawContexts.
  How do we determine what all renderpasses a RenderConfig belongs to.
*/
