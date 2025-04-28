#pragma once

#include "r3/render-pass/PipelineCreateInfo.hpp"

namespace tr {

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
      -> std::tuple<vk::raii::PipelineLayout, vk::raii::Pipeline>;

private:
  std::shared_ptr<Device> device;
  std::shared_ptr<IShaderModuleFactory> shaderModuleFactory;

  auto createGraphicsPipeline(const PipelineCreateInfo& createInfo)
      -> std::tuple<vk::raii::PipelineLayout, vk::raii::Pipeline>;

  auto createComputePipeline(const PipelineCreateInfo& createInfo)
      -> std::tuple<vk::raii::PipelineLayout, vk::raii::Pipeline>;
};

}
