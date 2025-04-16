#pragma once

#include "dd/render-pass/AttachmentConfig.hpp"
namespace tr {

struct RenderPassCreateInfo {
  std::vector<AttachmentConfig> colorAttachments;
  std::optional<AttachmentConfig> depthAttachment = std::nullopt;
  std::optional<std::string> debugName = std::nullopt;
};

}
