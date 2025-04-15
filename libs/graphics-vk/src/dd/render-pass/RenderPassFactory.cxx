#include "dd/render-pass/RenderPassFactory.hpp"
#include "dd/render-pass/RenderPass.hpp"
#include "dd/render-pass/RenderPassConfig.hpp"
#include "gfx/IFrameManager.hpp"
#include "img/ImageManager.hpp"

namespace tr {

RenderPassFactory::RenderPassFactory(std::shared_ptr<IFrameManager> newFrameManager,
                                     std::shared_ptr<ImageManager> newImageManager)
    : frameManager{std::move(newFrameManager)}, imageManager{std::move(newImageManager)} {
}

auto RenderPassFactory::createRenderPass(const RenderPassCreateInfo& info)
    -> std::unique_ptr<RenderPass> {
  RenderPassConfig config{};

  if (info.colorAttachment) {
    config.colorHandle = resolveOrCreateImage(*info.colorAttachment);
    config.colorAttachmentInfo = makeAttachmentInfo(*info.colorAttachment, *config.colorHandle);
    config.extent = info.colorAttachment->extent;
  }

  if (info.depthAttachment) {
    config.depthHandle = resolveOrCreateImage(*info.depthAttachment);
    config.depthAttachmentInfo = makeAttachmentInfo(*info.depthAttachment, *config.depthHandle);
    if (config.extent.width == 0u) {
      config.extent = info.depthAttachment->extent;
    }
  }

  return std::make_unique<RenderPass>(config, imageManager);
}

auto RenderPassFactory::resolveOrCreateImage(const AttachmentInfo& info) -> Handle<ManagedImage> {
  if (logicalImageRegistry.contains(info.logicalName)) {
    return logicalImageRegistry.at(info.logicalName);
  }

  const auto handle = imageManager->createImage(ImageUsageProfile{
      .format = info.format,
      .usage = info.usage,
      .aspectMask = info.aspect,
      .extent = info.extent,
  });

  logicalImageRegistry.emplace(info.logicalName, handle);
  return handle;
}

auto RenderPassFactory::makeAttachmentInfo(const AttachmentInfo& info,
                                           Handle<ManagedImage> imageHandle)
    -> vk::RenderingAttachmentInfo {
  return vk::RenderingAttachmentInfo{.imageView =
                                         imageManager->getImage(imageHandle).getImageView(),
                                     .imageLayout = info.finalLayout,
                                     .loadOp = info.loadOp,
                                     .storeOp = info.storeOp,
                                     .clearValue = vk::ClearValue{}};
}

}
