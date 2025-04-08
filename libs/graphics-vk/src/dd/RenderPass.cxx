#include "RenderPass.hpp"
#include "task/Frame.hpp"

namespace tr {

RenderPass::RenderPass(std::shared_ptr<VkResourceManager> newResourceManager,
                       RenderPassConfig newConfig)
    : resourceManager{std::move(newResourceManager)}, config{std::move(newConfig)} {
}

[[nodiscard]] auto RenderPass::accepts([[maybe_unused]] const RenderConfig& config) const -> bool {
  return true;
}

auto RenderPass::addDrawContext([[maybe_unused]] RenderConfigHandle handle,
                                [[maybe_unused]] DrawContext* drawContext) -> void {
}

auto RenderPass::execute(const Frame* frame, vk::raii::CommandBuffer& cmdBuffer) -> void {
  auto colorAttachmentInfo = config.colorAttachmentInfo;
  if (colorAttachmentInfo) {
    const auto imageHandle = frame->getLogicalImage(*config.colorHandle);
    colorAttachmentInfo->imageView = resourceManager->getImageView(imageHandle);
  }

  auto depthAttachmentInfo = config.depthAttachmentInfo;
  if (depthAttachmentInfo) {
    const auto imageHandle = frame->getLogicalImage(*config.depthHandle);
    depthAttachmentInfo->imageView = resourceManager->getImageView(imageHandle);
  }

  const auto renderingInfo =
      vk::RenderingInfo{.renderArea = {.offset = {.x = 0, .y = 0}, .extent = config.extent},
                        .layerCount = 1,
                        .colorAttachmentCount = 1,
                        .pColorAttachments = &*colorAttachmentInfo,
                        .pDepthAttachment = &*depthAttachmentInfo};

  cmdBuffer.beginRendering(renderingInfo);

  // Render DrawContexts

  cmdBuffer.endRendering();
}

}
