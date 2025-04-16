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

auto RenderPassFactory::createRenderPass(RenderPassCreateInfo& info)
    -> std::unique_ptr<RenderPass> {
  RenderPassConfig config{};

  std::ranges::copy(config.colorAttachmentConfigs, std::back_inserter(info.colorAttachments));

  if (info.depthAttachment) {
    config.depthAttachmentConfig = info.depthAttachment;
  }

  return std::make_unique<RenderPass>(config, imageManager);
}

}
