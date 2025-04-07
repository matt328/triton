#include "DrawContext.hpp"

namespace tr {

auto DrawContext::registerRenderable([[maybe_unused]] const RenderableData& renderableData)
    -> RenderableResources {
  return {};
}

auto DrawContext::writeObjectData([[maybe_unused]] const Renderable& renderable) -> void {
}

}
