#pragma once

#include "bk/Handle.hpp"
#include "dd/render-pass/RenderPassCreateInfo.hpp"
#include "img/ManagedImage.hpp"

namespace tr {

class RenderPass;
class IFrameManager;
class ImageManager;

class RenderPassFactory {
public:
  RenderPassFactory(std::shared_ptr<IFrameManager> newFrameManager,
                    std::shared_ptr<ImageManager> newImageManager);
  ~RenderPassFactory() = default;

  RenderPassFactory(const RenderPassFactory&) = default;
  RenderPassFactory(RenderPassFactory&&) = delete;
  auto operator=(const RenderPassFactory&) -> RenderPassFactory& = default;
  auto operator=(RenderPassFactory&&) -> RenderPassFactory& = delete;

  auto createRenderPass(const RenderPassCreateInfo& createInfo) -> std::unique_ptr<RenderPass>;

private:
  std::shared_ptr<IFrameManager> frameManager;
  std::shared_ptr<ImageManager> imageManager;

  std::unordered_map<std::string, Handle<ManagedImage>> logicalImageRegistry;

  auto resolveOrCreateImage(const AttachmentInfo& info) -> Handle<ManagedImage>;
  auto makeAttachmentInfo(const AttachmentInfo& info, Handle<ManagedImage> imageHandle)
      -> vk::RenderingAttachmentInfo;
};

}
