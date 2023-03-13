#pragma once
#include "AbstractPipeline.h"

class DefaultPipeline final : public AbstractPipeline {
 public:
   explicit DefaultPipeline(const vk::raii::Device& device,
                            const vk::raii::RenderPass& renderPass,
                            const vk::Extent2D& swapchainExtent);
   ~DefaultPipeline() override;

 private:
   static vk::raii::DescriptorSetLayout createDescriptorSetLayout(const vk::raii::Device& device);
};
