#include <utility>

#include "RenderPass.hpp"
#include "img/ImageManager.hpp"
#include "task/Frame.hpp"

namespace tr {

RenderPass::RenderPass(RenderPassConfig newConfig, std::shared_ptr<ImageManager> newImageManager)
    : imageManager{std::move(newImageManager)}, config{std::move(newConfig)} {
}

[[nodiscard]] auto RenderPass::accepts([[maybe_unused]] const RenderConfig& config) const -> bool {
  return true;
}

/*
  Note: the list of DrawContexts needs to be unique, this will potentially be called multiple times
  with the same DrawContext. I think we can uniquely identify a DrawContext by its Id
*/
auto RenderPass::addDrawContext([[maybe_unused]] RenderConfigHandle handle,
                                [[maybe_unused]] DrawContext* drawContext) -> void {
}

auto RenderPass::execute(const Frame* frame, vk::raii::CommandBuffer& cmdBuffer) -> void {
  std::vector<vk::RenderingAttachmentInfo> colorAttachments;

  // Process color attachments
  for (const auto& attachmentConfig : config.colorAttachmentConfigs) {
    const auto imageHandle = frame->getLogicalImage(attachmentConfig.logicalImage);
    const auto& imageView = imageManager->getImage(imageHandle).getImageView();
    const auto layout = vk::ImageLayout::eColorAttachmentOptimal;

    colorAttachments.emplace_back(vk::RenderingAttachmentInfo{
        .imageView = imageView,
        .imageLayout = layout,
        .loadOp = attachmentConfig.loadOp,
        .storeOp = attachmentConfig.storeOp,
        .clearValue = attachmentConfig.clearValue.value_or(vk::ClearValue{})});
  }

  // Process depth/stencil attachment (only one allowed in dynamic rendering)
  std::optional<vk::RenderingAttachmentInfo> depthAttachment;
  if (config.depthAttachmentConfig) {
    const auto& depthConfig = *config.depthAttachmentConfig;
    const auto imageHandle = frame->getLogicalImage(depthConfig.logicalImage);
    const auto& imageView = imageManager->getImage(imageHandle).getImageView();
    const auto layout = vk::ImageLayout::eDepthAttachmentOptimal;

    depthAttachment = vk::RenderingAttachmentInfo{
        .imageView = imageView,
        .imageLayout = layout,
        .loadOp = depthConfig.loadOp,
        .storeOp = depthConfig.storeOp,
        .clearValue = depthConfig.clearValue.value_or(vk::ClearValue{})};
  }

  // Build rendering info
  vk::RenderingInfo renderingInfo{
      .renderArea = {.offset = {.x = 0, .y = 0}, .extent = config.extent},
      .layerCount = 1,
      .colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size()),
      .pColorAttachments = colorAttachments.data(),
      .pDepthAttachment = depthAttachment ? &*depthAttachment : nullptr};

  cmdBuffer.beginRendering(renderingInfo);

  for (auto& drawContext : drawContexts) {
    drawContext.second->record(cmdBuffer);
  }

  cmdBuffer.endRendering();
}

}
