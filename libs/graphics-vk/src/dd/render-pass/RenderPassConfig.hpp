#pragma once

#include "dd/render-pass/RenderPassCreateInfo.hpp"

namespace tr {

struct RenderPassConfig {
  std::vector<AttachmentConfig> colorAttachmentConfigs;
  std::optional<AttachmentConfig> depthAttachmentConfig;
  vk::Extent2D extent;
  std::optional<std::string> debugName;
};

}
