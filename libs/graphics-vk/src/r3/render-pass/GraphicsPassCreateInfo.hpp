#pragma once

#include "bk/Handle.hpp"
#include "img/ManagedImage.hpp"
#include "r3/render-pass/PipelineCreateInfo.hpp"

namespace tr {

struct ImageUsageInfo {
  LogicalHandle<ManagedImage> imageHandle;
  vk::Format imageFormat;
  vk::AccessFlags accessFlags;
  vk::PipelineStageFlags stageFlags;
  vk::ImageAspectFlags aspectFlags;
  vk::ClearValue clearValue;
};

struct AttachmentCreateInfo {
  vk::Format format;
  std::optional<vk::ClearValue> clearValue = std::nullopt;
};

struct GraphicsPassCreateInfo {
  std::string id;
  PipelineLayoutInfo pipelineLayoutInfo;

  std::vector<ImageUsageInfo> inputs;
  std::vector<ImageUsageInfo> outputs;

  std::vector<ShaderStageInfo> shaderStageInfo;

  vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
  vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
  vk::CullModeFlagBits cullMode = vk::CullModeFlagBits::eBack;
  bool enableDepthTest = true;
  bool enableDepthWrite = true;

  vk::Extent2D extent;
};
}
