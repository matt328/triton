#pragma once

#include "r3/render-pass/AttachmentConfig.hpp"

namespace tr {

struct GraphicsPassConfig {
  std::string id;
  vk::raii::Pipeline pipeline;
  vk::raii::PipelineLayout pipelineLayout;
  std::optional<std::string> debugName = std::nullopt;

  std::vector<AttachmentConfig> colorAttachmentConfigs;
  std::optional<AttachmentConfig> depthAttachmentConfig = std::nullopt;
  vk::Extent2D extent;
};

}
