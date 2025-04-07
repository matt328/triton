#include "dd/DDRenderer.hpp"
#include "dd/DrawContextFactory.hpp"
#include "dd/RenderConfigRegistry.hpp"
#include "dd/RenderPassBuilder.hpp"
#include "gfx/IFrameManager.hpp"
#include "vk/core/Swapchain.hpp"
#include "task/Frame.hpp"

namespace tr {

DDRenderer::DDRenderer(std::shared_ptr<RenderConfigRegistry> newRenderConfigRegistry,
                       std::shared_ptr<DrawContextFactory> newDrawContextFactory,
                       std::shared_ptr<IFrameManager> newFrameManager)
    : renderConfigRegistry{std::move(newRenderConfigRegistry)},
      drawContextFactory{std::move(newDrawContextFactory)},
      frameManager{std::move(newFrameManager)} {

  renderPasses.emplace(RenderPassType::ForwardOpaque, std::make_unique<RenderPass>());
  renderPasses.emplace(RenderPassType::UI, std::make_unique<RenderPass>());
}

auto DDRenderer::update() -> void {
  // Not really used. Can probably get rid of it
}

auto DDRenderer::registerRenderable(const RenderableData& data) -> RenderableResources {
  const auto renderConfig = RenderConfig{.vertexFormat = data.geometryData.getVertexList().format,
                                         .topology = data.geometryData.getVertexList().topology,
                                         .shadingMode = data.materialData.shadingMode};
  const auto renderConfigHandle = renderConfigRegistry->registerOrGet(renderConfig);

  auto* const drawContext = drawContextFactory->getOrCreateDrawContext(renderConfigHandle);

  for (const auto& [renderPassType, renderPass] : renderPasses) {
    if (renderPass->accepts(renderConfig)) {
      renderPass->addDrawContext(renderConfigHandle, drawContext);
    }
  }

  return drawContext->registerRenderable(data);
}

auto DDRenderer::renderNextFrame() -> void {

  Log.trace("DDRenderer rendering next frame");

  const auto result = frameManager->acquireFrame();

  if (std::holds_alternative<ImageAcquireResult>(result)) {
    if (const auto acquireResult = std::get<ImageAcquireResult>(result);
        acquireResult == ImageAcquireResult::Error) {
      Log.warn("Failed to acquire swapchain image");
      return;
    }
  }
  if (std::holds_alternative<Frame*>(result)) {
    /*
    auto* frame = std::get<Frame*>(result);

    for (const auto& [type, pass] : renderPasses) {
      pass->execute(frame, nullptr);
    }
      */
  }
}

auto DDRenderer::waitIdle() -> void {
}

auto DDRenderer::setRenderData(const RenderData& newRenderData) -> void {
  renderData = newRenderData;
}

}
