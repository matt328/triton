#pragma once

#include "dd/render-pass/AttachmentConfig.hpp"
#include "dd/render-pass/RenderPassType.hpp"

namespace tr {

struct ShaderStageInfo {
  vk::ShaderStageFlagBits stage;
  std::string entryPoint = "main";
  std::filesystem::path shaderFile;
};

struct PushConstantRangeInfo {
  vk::ShaderStageFlags stages;
  uint32_t offset;
  uint32_t size;
};

struct PipelineLayoutInfo {
  std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
  std::vector<PushConstantRangeInfo> pushConstantRanges;
};

struct AttachmentInfo {
  std::vector<vk::Format> colorAttachmentFormats;
  std::optional<vk::Format> depthAttachmentFormat;
};

struct GraphicsPipelineInfo {
  PipelineLayoutInfo layout;

  AttachmentInfo attachmentInfo;

  std::vector<ShaderStageInfo> shaderStageInfo;

  std::vector<vk::VertexInputAttributeDescription> vertexAttributes;
  std::vector<vk::VertexInputBindingDescription> vertexBindings;

  vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
  vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
  vk::CullModeFlags cullMode = vk::CullModeFlagBits::eBack;

  vk::Extent2D extent;

  bool enableDepthTest = true;
  bool enableDepthWrite = true;
  bool enableBlending = false;
};

struct ComputePipelineInfo {
  ShaderStageInfo shader;
  PipelineLayoutInfo layout;
};

struct RenderPassCreateInfo {
  RenderPassType passType;

  std::optional<ComputePipelineInfo> computePipelineInfo;
  std::optional<GraphicsPipelineInfo> graphicsPipelineInfo;

  std::optional<std::string> debugName;
};

}
