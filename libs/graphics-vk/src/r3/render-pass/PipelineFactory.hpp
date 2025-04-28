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
  std::string identifier;
  PipelineType pipelineType;
  PipelineLayoutInfo pipelineLayoutInfo;
  std::vector<vk::Format> colorAttachmentFormats;
  std::optional<vk::Format> depthAttachmentFormat;
  std::vector<ShaderStageInfo> shaderStageInfo;
  vk::PrimitiveTopology topology;
  vk::PolygonMode polygonMode;
  vk::CullModeFlagBits cullMode;
  bool enableDepthTest;
  bool enableDepthWrite;
};

class Device;
class IShaderModuleFactory;

class PipelineFactory {
public:
  PipelineFactory(std::shared_ptr<Device> newDevice,
                  std::shared_ptr<IShaderModuleFactory> newShaderModuleFactory);
  ~PipelineFactory() = default;

  PipelineFactory(const PipelineFactory&) = default;
  PipelineFactory(PipelineFactory&&) = delete;
  auto operator=(const PipelineFactory&) -> PipelineFactory& = default;
  auto operator=(PipelineFactory&&) -> PipelineFactory& = delete;

  auto createPipeline(const PipelineCreateInfo& createInfo)
      -> std::tuple<std::unique_ptr<vk::raii::PipelineLayout>, std::unique_ptr<vk::raii::Pipeline>>;

private:
  std::shared_ptr<Device> device;
  std::shared_ptr<IShaderModuleFactory> shaderModuleFactory;

  auto createGraphicsPipeline(const PipelineCreateInfo& createInfo)
      -> std::tuple<std::unique_ptr<vk::raii::PipelineLayout>, std::unique_ptr<vk::raii::Pipeline>>;

  auto createComputePipeline(const PipelineCreateInfo& createInfo)
      -> std::tuple<std::unique_ptr<vk::raii::PipelineLayout>, std::unique_ptr<vk::raii::Pipeline>>;
};

}
