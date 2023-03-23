#pragma once
#include "AbstractPipeline.hpp"

class DefaultPipeline final : public AbstractPipeline {
 public:
   explicit DefaultPipeline(const vk::raii::Device& device,
                            const vk::raii::RenderPass& renderPass,
                            const vk::Extent2D& swapchainExtent);
   ~DefaultPipeline() override = default;

 private:
   static vk::raii::DescriptorSetLayout createDescriptorSetLayout(const vk::raii::Device& device);
};
