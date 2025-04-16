#pragma once

#include "dd/render-pass/RenderPassCreateInfo.hpp"

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

  auto createRenderPass(RenderPassCreateInfo& createInfo) -> std::unique_ptr<RenderPass>;

private:
  std::shared_ptr<IFrameManager> frameManager;
  std::shared_ptr<ImageManager> imageManager;
};

}
