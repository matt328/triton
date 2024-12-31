#include "ComputePipeline.hpp"
#include "cm/IndirectPushConstants.hpp"
#include "vk/Device.hpp"
#include "pipeline/IShaderCompiler.hpp"

namespace tr {

const auto ShaderFile = SHADERS / "compute.comp";

ComputePipeline::ComputePipeline(const std::shared_ptr<Device>& device,
                                 const std::shared_ptr<IShaderCompiler>& shaderCompiler) {
  Log.trace("Constructing ComputePipeline");

  const auto pushConstantRange = vk::PushConstantRange{
      .stageFlags = vk::ShaderStageFlagBits::eCompute,
      .offset = 0,
      .size = sizeof(ComputePushConstants),
  };

  const auto pipelineLayoutCreateInfo =
      vk::PipelineLayoutCreateInfo{.pushConstantRangeCount = 1,
                                   .pPushConstantRanges = &pushConstantRange};

  pipelineLayout =
      std::make_unique<vk::raii::PipelineLayout>(device->getVkDevice(), pipelineLayoutCreateInfo);

  auto csm = shaderCompiler->createShaderModule(vk::ShaderStageFlagBits::eCompute, ShaderFile);
  auto shaderStage = vk::PipelineShaderStageCreateInfo{.stage = vk::ShaderStageFlagBits::eCompute,
                                                       .module = *csm,
                                                       .pName = "main"};

  const auto pipelineCreateInfo = vk::ComputePipelineCreateInfo{
      .stage = shaderStage,
      .layout = *pipelineLayout,
  };

  pipeline = std::make_unique<vk::raii::Pipeline>(device->getVkDevice(),
                                                  VK_NULL_HANDLE,
                                                  pipelineCreateInfo);
}

auto ComputePipeline::getPipeline() const -> vk::Pipeline {
  return *pipeline;
}

auto ComputePipeline::getPipelineLayout() const -> vk::PipelineLayout {
  return *pipelineLayout;
}

}
