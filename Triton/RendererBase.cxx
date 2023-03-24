#include "pch.hpp"

#include "RendererBase.hpp"

RendererBase::RendererBase(const RendererBaseCreateInfo& createInfo)
    : device(createInfo.device),
      allocator(createInfo.allocator),
      depthTexture(createInfo.depthTexture),
      framebufferSize(createInfo.swapchainExtent),
      framesInFlight(createInfo.swapchainImages.size()),
      swapchainImages(createInfo.swapchainImages) {
  const auto poolSize = std::array{
      vk::DescriptorPoolSize{
          .type = vk::DescriptorType::eUniformBuffer,
          .descriptorCount = static_cast<uint32_t>(framesInFlight) * 10},
      vk::DescriptorPoolSize{
          .type = vk::DescriptorType::eCombinedImageSampler,
          .descriptorCount = static_cast<uint32_t>(framesInFlight) * 10}};

  const vk::DescriptorPoolCreateInfo poolInfo{
      .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      .maxSets = static_cast<uint32_t>(framesInFlight) * 2,
      .poolSizeCount = 2,
      .pPoolSizes = poolSize.data()};

  descriptorPool =
      std::make_unique<vk::raii::DescriptorPool>(device.createDescriptorPool(poolInfo, nullptr));
}

void RendererBase::beginRenderPass(
    const vk::raii::CommandBuffer& commandBuffer,
    const size_t currentImage,
    const vk::raii::DescriptorSet& currentDescriptorSet) const {

  const auto screenRect = vk::Rect2D{.offset = {0, 0}, .extent = framebufferSize};
  const auto renderPassBeginInfo = vk::RenderPassBeginInfo{
      .renderPass = **renderPass,
      .framebuffer = **swapchainFramebuffers[currentImage],
      .renderArea = screenRect};

  commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, **pipeline);
  commandBuffer.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, **pipelineLayout, 0, *currentDescriptorSet, nullptr);
}

void RendererBase::createUniformBuffers(const size_t bufferSize) {
  uniformBuffers.resize(framesInFlight);
  const auto bufferCreateInfo = vk::BufferCreateInfo{
      .size = bufferSize,
      .usage = vk::BufferUsageFlagBits::eUniformBuffer,
  };
  constexpr auto allocationCreateInfo = vma::AllocationCreateInfo{
      .usage = vma::MemoryUsage::eCpuToGpu,
      .requiredFlags = vk::MemoryPropertyFlagBits::eHostCoherent};

  for (size_t i = 0; i < framesInFlight; i++) {
    uniformBuffers.emplace_back(allocator.createBuffer(&bufferCreateInfo, &allocationCreateInfo));
  }
}
