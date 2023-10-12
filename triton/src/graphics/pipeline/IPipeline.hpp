#pragma once

namespace Triton {

   class IPipeline {
    public:
      IPipeline(const IPipeline&) = delete;
      IPipeline(IPipeline&&) = delete;
      IPipeline& operator=(const IPipeline&) = delete;
      IPipeline& operator=(IPipeline&&) = delete;
      virtual ~IPipeline() = default;

      [[nodiscard]] virtual const vk::raii::Pipeline& getPipeline() const = 0;
      [[nodiscard]] virtual const vk::raii::DescriptorSetLayout& getDescriptorSetLayout() const = 0;
      [[nodiscard]] virtual const vk::raii::PipelineLayout& getPipelineLayout() const = 0;
   };
}