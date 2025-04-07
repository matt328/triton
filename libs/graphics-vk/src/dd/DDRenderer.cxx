#include "dd/DDRenderer.hpp"
#include "dd/DrawContextFactory.hpp"
#include "dd/RenderConfigRegistry.hpp"
#include "gfx/IFrameManager.hpp"
#include "vk/core/Swapchain.hpp"
#include "task/Frame.hpp"
#include "dd/RenderPassFactory.hpp"

namespace tr {

DDRenderer::DDRenderer(std::shared_ptr<RenderConfigRegistry> newRenderConfigRegistry,
                       std::shared_ptr<DrawContextFactory> newDrawContextFactory,
                       std::shared_ptr<IFrameManager> newFrameManager,
                       std::shared_ptr<RenderPassFactory> newRenderPassFactory)
    : renderConfigRegistry{std::move(newRenderConfigRegistry)},
      drawContextFactory{std::move(newDrawContextFactory)},
      frameManager{std::move(newFrameManager)},
      renderPassFactory{std::move(newRenderPassFactory)} {

  const auto forwardCreateInfo = RenderPassCreateInfo{};
  const auto uiCreateInfo = RenderPassCreateInfo{};

  renderPasses.emplace_back(renderPassFactory->createRenderPass(forwardCreateInfo));
  renderPasses.emplace_back(renderPassFactory->createRenderPass(uiCreateInfo));
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

  for (const auto& renderPass : renderPasses) {
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
