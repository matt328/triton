#pragma once

#include "dd/render-pass/AttachmentConfig.hpp"

namespace tr {

struct GraphicsPassConfig {
  vk::raii::Pipeline pipeline;
  vk::raii::PipelineLayout pipelineLayout;
  std::optional<std::string> debugName = std::nullopt;

  std::vector<AttachmentConfig> colorAttachmentConfigs;
  std::optional<AttachmentConfig> depthAttachmentConfig;
  vk::Extent2D extent;
};

}
