#include "IndirectRenderTask.hpp"
#include "gfx/RenderContextConfig.hpp"
#include "task/Frame.hpp"
#include "vk/VkResourceManager.hpp"
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace tr {

IndirectRenderTask::IndirectRenderTask(std::shared_ptr<VkResourceManager> newResourceManager,
                                       std::shared_ptr<IndirectPipeline> newPipeline,
                                       RenderContextConfig newConfig)
    : resourceManager{std::move(newResourceManager)},
      pipeline{std::move(newPipeline)},
      config{newConfig} {
}

auto IndirectRenderTask::record(vk::raii::CommandBuffer& commandBuffer,
                                const Frame& frame) -> void {

  const auto& objectDataBuffer = resourceManager->getBuffer(frame.getGpuObjectDataBufferHandle());
  const auto& cameraDataBuffer = resourceManager->getBuffer(frame.getCameraBufferHandle());

  pushConstants = IndirectPushConstants{.drawID = 0,
                                        .objectDataAddress = objectDataBuffer.getDeviceAddress(),
                                        .cameraDataAddress = cameraDataBuffer.getDeviceAddress()};

  // Bind the graphics pipeline
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->getPipeline());

  const auto& [vertexBuffer, indexBuffer] = resourceManager->getStaticMeshBuffers();
  commandBuffer.bindVertexBuffers(0, vertexBuffer.getBuffer(), {0});
  commandBuffer.bindIndexBuffer(indexBuffer.getBuffer(), 0, vk::IndexType::eUint32);

  const auto& descriptorBuffer = resourceManager->getDescriptorBuffer();

  const auto address = descriptorBuffer.getDeviceAddress();
  // TODO(Matt): Buffer::getUsage()
  const auto usage = vk::BufferUsageFlagBits::eResourceDescriptorBufferEXT |
                     vk::BufferUsageFlagBits::eSamplerDescriptorBufferEXT |
                     vk::BufferUsageFlagBits::eShaderDeviceAddress;

  const auto bindingInfo = vk::DescriptorBufferBindingInfoEXT{.address = address, .usage = usage};

  commandBuffer.bindDescriptorBuffersEXT({bindingInfo});

  commandBuffer.setDescriptorBufferOffsetsEXT(vk::PipelineBindPoint::eGraphics,
                                              pipeline->getPipelineLayout(),
                                              0,
                                              {0},
                                              {0});

  commandBuffer.pushConstants<IndirectPushConstants>(pipeline->getPipelineLayout(),
                                                     vk::ShaderStageFlagBits::eVertex,
                                                     0,
                                                     pushConstants);

  auto& indirectBuffer = resourceManager->getBuffer(frame.getDrawCommandBufferHandle());
  auto& countBuffer = resourceManager->getBuffer(frame.getCountBufferHandle());

  commandBuffer.drawIndexedIndirectCount(indirectBuffer.getBuffer(),
                                         0,
                                         countBuffer.getBuffer(),
                                         0,
                                         config.maxStaticObjects,
                                         sizeof(vk::DrawIndexedIndirectCommand));
}

}
