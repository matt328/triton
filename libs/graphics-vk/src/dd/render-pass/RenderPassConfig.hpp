#pragma once

#include "dd/render-pass/AttachmentConfig.hpp"
#include "dd/render-pass/RenderPassType.hpp"

namespace tr {

struct ComputePipelineConfig {
  vk::raii::Pipeline pipeline;
  vk::raii::PipelineLayout pipelineLayout;
};

struct GraphicsPipelineConfig {
  vk::raii::Pipeline pipeline;
  vk::raii::PipelineLayout pipelineLayout;
  std::vector<AttachmentConfig> colorAttachmentConfigs;
  std::optional<AttachmentConfig> depthAttachmentConfig;
  vk::Extent2D extent;
};

struct RenderPassConfig {
  RenderPassType passType;

  std::optional<GraphicsPipelineConfig> graphicsConfig;
  std::optional<ComputePipelineConfig> computeConfig;

  std::optional<std::string> debugName;
};

}
