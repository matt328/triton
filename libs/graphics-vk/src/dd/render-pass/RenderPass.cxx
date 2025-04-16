#include <utility>

#include "RenderPass.hpp"
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
  std::optional<vk::RenderingAttachmentInfo> depthAttachment;

  // Process color attachments
  for (const auto& attachmentConfig : config.colorAttachmentConfigs) {
    const auto imageHandle = frame->getLogicalImage(attachmentConfig.imageHandle);
    auto imageView = imageManager->getImageView(imageHandle);

    vk::RenderingAttachmentInfo attachment{.imageView = *imageView,
                                           .imageLayout = info.imageLayout,
                                           .loadOp = info.loadOp,
                                           .storeOp = info.storeOp,
                                           .clearValue = info.clearValue};

    colorAttachments.push_back(attachment);
  }

  // Process depth/stencil attachment (only one allowed in dynamic rendering)
  if (config.depthAttachment) {
    const auto& info = *config.depthAttachment;
    const auto imageHandle = frame->getLogicalImage(info.imageHandle);
    auto imageView = imageManager->getImageView(imageHandle);

    depthAttachment = vk::RenderingAttachmentInfo{.imageView = *imageView,
                                                  .imageLayout = info.imageLayout,
                                                  .loadOp = info.loadOp,
                                                  .storeOp = info.storeOp,
                                                  .clearValue = info.clearValue};
  }

  // Build rendering info
  vk::RenderingInfo renderingInfo{
      .renderArea = {.offset = {.x = 0, .y = 0}, .extent = config.extent},
      .layerCount = 1,
      .colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size()),
      .pColorAttachments = colorAttachments.data(),
      .pDepthAttachment = depthAttachment ? &*depthAttachment : nullptr};

  cmdBuffer.beginRendering(renderingInfo);

  // --- Render all draw contexts here ---
  for (auto& drawContext : drawContexts) {
    drawContext.render(cmdBuffer); // Or whatever your draw context does
  }

  cmdBuffer.endRendering();
}

}
