#pragma once
#include "AbstractPipeline.h"

class DefaultPipeline final : public AbstractPipeline {
 public:
   explicit DefaultPipeline(const vk::raii::Device& device,
                            const vk::raii::RenderPass& renderPass,
                            const vk::raii::DescriptorSetLayout& descriptorSetLayout,
                            const vk::Extent2D& swapchainExtent);
   ~DefaultPipeline() override;

   static vk::raii::DescriptorSetLayout createDescriptorSetLayout(const vk::raii::Device& device);
};
