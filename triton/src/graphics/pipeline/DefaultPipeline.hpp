#pragma once
#include "AbstractPipeline.hpp"

namespace Triton {
   class DefaultPipeline final : public AbstractPipeline {
    public:
      explicit DefaultPipeline(const vk::raii::Device& device,
                               const vk::raii::RenderPass& renderPass,
                               const vk::Extent2D& swapchainExtent);

      DefaultPipeline(const DefaultPipeline&) = delete;
      DefaultPipeline(DefaultPipeline&&) = delete;
      DefaultPipeline& operator=(const DefaultPipeline&) = delete;
      DefaultPipeline& operator=(DefaultPipeline&&) = delete;

      ~DefaultPipeline() override = default;

      [[nodiscard]] const vk::raii::PipelineLayout& getPipelineLayout() const override {
         return *pipelineLayout;
      }

    private:
      static vk::raii::DescriptorSetLayout createDescriptorSetLayout(
          const vk::raii::Device& device);
      std::unique_ptr<vk::raii::PipelineLayout> pipelineLayout;
   };
}