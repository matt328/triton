#include "dd/RenderPassFactory.hpp"
#include "VkResourceManager.hpp"
#include "dd/RenderPass.hpp"
#include "gfx/IFrameManager.hpp"

namespace tr {

RenderPassFactory::RenderPassFactory(std::shared_ptr<IFrameManager> newFrameManager,
                                     std::shared_ptr<VkResourceManager> newResourceManager)
    : frameManager{std::move(newFrameManager)}, resourceManager{std::move(newResourceManager)} {
}

auto RenderPassFactory::createRenderPass(const RenderPassCreateInfo& createInfo)
    -> std::unique_ptr<RenderPass> {
  auto logicalColorHandle = std::optional<LogicalImageHandle>{};
  auto logicalDepthHandle = std::optional<LogicalImageHandle>{};
  auto colorAttachmentInfo = std::optional<vk::RenderingAttachmentInfo>{};
  auto depthAttachmentInfo = std::optional<vk::RenderingAttachmentInfo>{};

  if (createInfo.colorFormat) {
    logicalColorHandle = frameManager->registerPerFrameDrawImage(createInfo.extent);
    colorAttachmentInfo =
        std::make_optional<vk::RenderingAttachmentInfo>(vk::RenderingAttachmentInfo{
            .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .loadOp = createInfo.colorLoadOp,
            .storeOp = createInfo.colorStoreOp,
            .clearValue = vk::ClearValue{.color = vk::ClearColorValue{*createInfo.clearColor}}});
  }

  if (createInfo.depthFormat) {
    // todo registerPerFrameImage(extent, format)
    logicalDepthHandle =
        frameManager->registerPerFrameDepthImage(createInfo.extent, *createInfo.depthFormat);
    depthAttachmentInfo =
        std::make_optional<vk::RenderingAttachmentInfo>(vk::RenderingAttachmentInfo{
            .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
            .loadOp = createInfo.depthLoadOp,
            .storeOp = createInfo.depthStoreOp,
            .clearValue = vk::ClearValue{
                .depthStencil = vk::ClearDepthStencilValue{.depth = *createInfo.clearDepth,
                                                           .stencil = *createInfo.clearStencil}}});
  }

  return std::make_unique<RenderPass>(resourceManager,
                                      RenderPassConfig{.extent = createInfo.extent,
                                                       .colorHandle = logicalColorHandle,
                                                       .depthHandle = logicalDepthHandle,
                                                       .colorAttachmentInfo = colorAttachmentInfo,
                                                       .depthAttachmentInfo = depthAttachmentInfo});
}

}
