#include "dd/DDRenderer.hpp"

namespace tr {

auto DDRenderer::update() -> void {
  // Not really used. Can probably get rid of it
}

auto DDRenderer::renderNextFrame() -> void {
  for (const auto& renderable : renderables) {
    categorizedRenderables[renderable.renderConfigHandle].push_back(renderable);
  }

  for (auto& [configHandle, renderableList] : categorizedRenderables) {
    const auto& drawContext = getDrawContext(configHandle);

    for (const auto& renderable : renderableList) {
      drawContext.objectDataBuffer.push(renderable.objectId, renderable.transform);
    }

    dispatchCompute(drawContext);
  }

  for (auto& [configHandle, drawContext] : activeDrawContexts) {
    bindPipeline(drawContext.pipeline);
    bindDescriptorSets(drawContext.descriptorSet);
    bindVertexBuffer(drawContext.vertexBuffer);
    bindIndexBuffer(drawContext.indexBuffer);

    draw(commandBuffer,
         drawContext.indirectBuffer,
         0,
         drawContext.countBuffer,
         0,
         drawContext.maxDrawCount,
         sizeof(VkDrawIndexedIndirectCommand));
  }
}

auto DDRenderer::waitIdle() -> void {
}

auto DDRenderer::setRenderData(const RenderData& newRenderData) -> void {
  renderData = newRenderData;
}

}
