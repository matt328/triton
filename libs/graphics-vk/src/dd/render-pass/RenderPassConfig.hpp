#pragma once

#include "bk/Handle.hpp"
#include "img/ManagedImage.hpp"

namespace tr {

struct AttachmentConfig {
  Handle<ManagedImage> imageHandle;
  vk::ClearValue clearValue;
  vk::AttachmentLoadOp loadOp;
  vk::AttachmentStoreOp storeOp;
  vk::ImageLayout layout;
  vk::ImageAspectFlags aspectMask;
};

struct RenderPassConfig {
  std::vector<AttachmentConfig> colorAttachmentConfigs;
  std::optional<AttachmentConfig> depthAttachmentConfig;
  vk::Extent2D extent;
  std::optional<std::string> debugName;
};

}
