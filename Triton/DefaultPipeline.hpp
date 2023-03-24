#pragma once
#include "AbstractPipeline.hpp"

class DefaultPipeline final : public AbstractPipeline {
 public:
   explicit DefaultPipeline(const vk::raii::Device& device,
                            const vk::raii::RenderPass& renderPass,
                            const vk::Extent2D& swapchainExtent);
   ~DefaultPipeline() override;

   const vk::raii::PipelineLayout& getPipelineLayout() const override {
      return *pipelineLayout;
   }

 private:
   static vk::raii::DescriptorSetLayout createDescriptorSetLayout(const vk::raii::Device& device);
   std::unique_ptr<vk::raii::PipelineLayout> pipelineLayout;
};
