#include "dd/DDRenderer.hpp"

namespace tr {

auto DDRenderer::update() -> void {
  // Not really used. Can probably get rid of it
}

auto DDRenderer::renderNextFrame() -> void {
}

auto DDRenderer::waitIdle() -> void {
}

auto DDRenderer::setRenderData(const RenderData& newRenderData) -> void {
  renderData = newRenderData;
}

}
