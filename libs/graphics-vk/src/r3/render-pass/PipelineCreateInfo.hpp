#pragma once

namespace tr {
enum class PipelineType : uint8_t {
  Compute = 0u,
  Graphics
};

struct PushConstantInfo {
  vk::ShaderStageFlags stageFlags;
  uint32_t offset;
  uint32_t size;
};

struct PipelineLayoutInfo {
  std::vector<PushConstantInfo> pushConstantInfoList;
  std::span<vk::DescriptorSetLayout> descriptorSetLayouts;
};

struct ShaderStageInfo {
  vk::ShaderStageFlagBits stage;
  std::string shaderFile;
  std::string entryPoint;
};

struct PipelineCreateInfo {
  std::string id;
  PipelineType pipelineType;
  PipelineLayoutInfo pipelineLayoutInfo;
  std::vector<vk::Format> colorAttachmentFormats;
  std::optional<vk::Format> depthAttachmentFormat = std::nullopt;
  std::vector<ShaderStageInfo> shaderStageInfo;

  vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
  vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
  vk::CullModeFlagBits cullMode = vk::CullModeFlagBits::eBack;
  bool enableDepthTest = true;
  bool enableDepthWrite = true;
};
}
