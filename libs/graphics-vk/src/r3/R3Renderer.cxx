#include "R3Renderer.hpp"
#include "api/gw/RenderableResources.hpp"

namespace tr {

auto R3Renderer::registerRenderable(const RenderableData& data) -> RenderableResources {
  // This won't actually need to be here. The Renderer will already 'expect' the things that can be
  // rendered
}

void R3Renderer::update() {
}

void R3Renderer::renderNextFrame() {
}

void R3Renderer::waitIdle() {
}

void R3Renderer::setRenderData(const RenderData& renderData) {
}

}
