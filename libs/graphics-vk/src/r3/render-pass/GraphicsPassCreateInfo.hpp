#pragma once

#include "r3/render-pass/PipelineCreateInfo.hpp"

namespace tr {

struct AttachmentCreateInfo {
  vk::Format format;
  std::optional<vk::ClearValue> clearValue = std::nullopt;
};

struct GraphicsPassCreateInfo {
  std::string id;
  PipelineLayoutInfo pipelineLayoutInfo;
  std::vector<AttachmentCreateInfo> colorAttachmentInfos;
  std::optional<AttachmentCreateInfo> depthAttachmentFormat = std::nullopt;
  std::vector<ShaderStageInfo> shaderStageInfo;

  vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
  vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
  vk::CullModeFlagBits cullMode = vk::CullModeFlagBits::eBack;
  bool enableDepthTest = true;
  bool enableDepthWrite = true;

  vk::Extent2D extent;
};
}
