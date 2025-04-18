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

struct GraphicsPipelineInfo {
  std::vector<ShaderStageInfo> shaders;
  vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
  bool enableDepthTest = true;
  bool enableDepthWrite = true;
  bool enableBlending = false;
  PipelineLayoutInfo layout;
};

struct ComputePipelineInfo {
  ShaderStageInfo shader;
  PipelineLayoutInfo layout;
};

struct RenderPassCreateInfo {
  RenderPassType passType;

  std::optional<ComputePipelineInfo> computePipelineInfo;

  std::vector<ShaderStageInfo> shaderStages;

  std::vector<vk::VertexInputAttributeDescription> vertexAttributes;
  std::vector<vk::VertexInputBindingDescription> vertexBindings;

  std::vector<AttachmentConfig> colorAttachments;
  std::optional<AttachmentConfig> depthAttachment = std::nullopt;

  vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
  vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
  vk::CullModeFlags cullMode = vk::CullModeFlagBits::eBack;

  vk::Extent2D extent; // optional, maybe dynamic later
  std::optional<std::string> debugName;
};

}
