#include "RendererBase.h"

RendererBase::RendererBase(const RendererBaseCreateInfo& createInfo)
    : device(createInfo.device)
    , depthTexture(createInfo.depthTexture)
    , framebufferSize(createInfo.swapchainExtent) {

   const auto framesInFlight = createInfo.swapchainImages.size();

   const auto poolSize =
       std::array{vk::DescriptorPoolSize{.type = vk::DescriptorType::eUniformBuffer,
                                         .descriptorCount = framesInFlight * 10},
                  vk::DescriptorPoolSize{.type = vk::DescriptorType::eCombinedImageSampler,
                                         .descriptorCount = framesInFlight * 10}};

   const vk::DescriptorPoolCreateInfo poolInfo{
       .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
       .maxSets = framesInFlight * 2,
       .poolSizeCount = 2,
       .pPoolSizes = poolSize.data()};

   descriptorPool =
       std::make_unique<vk::raii::DescriptorPool>(device.createDescriptorPool(poolInfo, nullptr));
}

void RendererBase::beginRenderPass(const vk::raii::CommandBuffer& commandBuffer,
                                   const size_t currentImage) const {
   const auto screenRect = vk::Rect2D{.offset = {0, 0}, .extent = framebufferSize};
   const auto renderPassBeginInfo =
       vk::RenderPassBeginInfo{.renderPass = **renderPass,
                               .framebuffer = **swapchainFramebuffers[currentImage],
                               .renderArea = screenRect};

   const auto& currentDescriptorSet = getDescriptorSet(currentImage);

   commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
   commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, **pipeline);
   commandBuffer.bindDescriptorSets(
       vk::PipelineBindPoint::eGraphics, **pipelineLayout, 0, *currentDescriptorSet, nullptr);
}

bool RendererBase::createUniformBuffers() {
}
