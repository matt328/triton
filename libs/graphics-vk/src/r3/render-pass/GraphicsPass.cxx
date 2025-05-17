#include "GraphicsPass.hpp"
#include "img/ImageManager.hpp"
#include "r3/draw-context/IDispatchContext.hpp"
#include "task/Frame.hpp"
#include "r3/draw-context/ContextFactory.hpp"

namespace tr {
GraphicsPass::GraphicsPass(GraphicsPassConfig&& config,
                           std::shared_ptr<ImageManager> newImageManager,
                           std::shared_ptr<ContextFactory> newDrawContextFactory)
    : imageManager{std::move(newImageManager)},
      drawContextFactory{std::move(newDrawContextFactory)},
      passConfig{std::move(config)} {

  renderingInfo = vk::RenderingInfo{
      .renderArea = vk::Rect2D{.offset = {.x = 0, .y = 0}, .extent = passConfig.extent},
      .layerCount = 1};

  colorAttachmentInfo.resize(passConfig.colorAttachmentConfigs.size());
  for (size_t i = 0; i < passConfig.colorAttachmentConfigs.size(); ++i) {
    const auto& attachmentConfig = passConfig.colorAttachmentConfigs[i];
    colorAttachmentInfo[i].setImageLayout(vk::ImageLayout::eColorAttachmentOptimal);
    colorAttachmentInfo[i].setClearValue(*attachmentConfig.clearValue);
    colorAttachmentInfo[i].setLoadOp(attachmentConfig.loadOp);
    colorAttachmentInfo[i].setStoreOp(attachmentConfig.storeOp);
  }
  if (passConfig.depthAttachmentConfig) {
    depthAttachmentInfo = std::make_optional(vk::RenderingAttachmentInfo{
        .imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
        .loadOp = passConfig.depthAttachmentConfig->loadOp,
        .storeOp = passConfig.depthAttachmentConfig->storeOp,
        .clearValue = passConfig.depthAttachmentConfig->clearValue.value_or(
            vk::ClearValue{.depthStencil = vk::ClearDepthStencilValue{.depth = 1.f, .stencil = 0}}),
    });
  }
}

auto GraphicsPass::execute(const Frame* frame, vk::raii::CommandBuffer& cmdBuffer) -> void {
  // Configure vk::RenderingInfo
  for (size_t i = 0; i < passConfig.colorAttachmentConfigs.size(); ++i) {
    const auto& handle = frame->getLogicalImage(passConfig.colorAttachmentConfigs[i].logicalImage);
    const auto& image = imageManager->getImage(handle);
    colorAttachmentInfo[i].setImageView(image.getImageView());
  }

  if (depthAttachmentInfo) {
    const auto& depthHandle =
        frame->getLogicalImage(passConfig.depthAttachmentConfig->logicalImage);
    const auto& depthImage = imageManager->getImage(depthHandle);
    depthAttachmentInfo->setImageView(depthImage.getImageView());
    renderingInfo.setPDepthAttachment(&(*depthAttachmentInfo));
  }

  renderingInfo.setColorAttachmentCount(colorAttachmentInfo.size());
  renderingInfo.setColorAttachments(colorAttachmentInfo);

  cmdBuffer.beginRendering(renderingInfo);
  cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, passConfig.pipeline);

  for (const auto& handle : drawableContexts) {
    const auto& drawContext = drawContextFactory->getDispatchContext(handle);
    drawContext->bind(frame, cmdBuffer, passConfig.pipelineLayout);
    drawContext->dispatch(frame, cmdBuffer);
  }

  cmdBuffer.endRendering();
}

auto GraphicsPass::registerDrawContext(Handle<IDispatchContext> handle) -> void {
  drawableContexts.push_back(handle);
}

}
