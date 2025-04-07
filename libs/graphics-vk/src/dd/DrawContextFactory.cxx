#include "dd/DrawContextFactory.hpp"
#include "dd/DrawContext.hpp"
#include "dd/RenderConfigRegistry.hpp"

namespace tr {

DrawContextFactory::DrawContextFactory(
    std::shared_ptr<RenderConfigRegistry> newRenderConfigRegistry)
    : renderConfigRegistry{std::move(newRenderConfigRegistry)} {
}

auto DrawContextFactory::getOrCreateDrawContext(RenderConfigHandle renderConfigHandle)
    -> DrawContext* {

  if (!drawContexts.contains(renderConfigHandle)) {
    const auto renderConfig = renderConfigRegistry->get(renderConfigHandle);

    // Get Vertex/Index buffer handles from BufferRegistry.

    // Get ObjectDataBuffer handles from BufferRegistry

    // Get Material handle from MaterialRegistry

    drawContexts.emplace(renderConfigHandle, std::make_unique<DrawContext>());
  }

  return drawContexts.at(renderConfigHandle).get();
}

}
