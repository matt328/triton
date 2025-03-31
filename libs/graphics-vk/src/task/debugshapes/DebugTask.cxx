#include "task/debugshapes/DebugTask.hpp"
#include "task/Frame.hpp"
#include "task/debugshapes/DebugPipeline.hpp"
#include "vk/BufferEntry.hpp"
#include "vk/BufferManager.hpp"
#include "api/gfx/GpuObjectData.hpp"

namespace tr {

DebugTask::DebugTask(std::shared_ptr<BufferManager> newBufferManager,
                     std::shared_ptr<DebugPipeline> newDebugPipeline,
                     std::shared_ptr<VkResourceManager> newResourceManager,
                     const RenderContextConfig& newRenderConfig)
    : bufferManager{std::move(newBufferManager)},
      pipeline{std::move(newDebugPipeline)},
      resourceManager{std::move(newResourceManager)},
      renderConfig(newRenderConfig) {
  Log.trace("Constructing DebugTask");
}

DebugTask::~DebugTask() {
  Log.trace("Destroying DebugTask");
}

auto DebugTask::record(vk::raii::CommandBuffer& commandBuffer, const Frame* frame) -> void {

  const auto objectDataAddress =
      bufferManager->getBuffer(frame->getBufferHandle2(objectDataBufferKey)).getDeviceAddress();
  const auto cameraDataAddress =
      bufferManager->getBuffer(frame->getBufferHandle(BufferHandleType::CameraBuffer))
          .getDeviceAddress();
  const auto& objectDataIndexAddress =
      bufferManager->getBuffer(frame->getBufferHandle2(objectDataIndexBufferKey))
          .getDeviceAddress();
  pushConstants = DebugPushConstants{.drawID = 0,
                                     .objectDataAddress = objectDataAddress,
                                     .cameraDataAddress = cameraDataAddress,
                                     .objectDataIndexAddress = objectDataIndexAddress,
                                     .objectCount = frame->getDebugObjectCount()};

  // Bind the graphics pipeline
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->getPipeline());

  // TODO(Matt): const auto meshBufferHandles = resourceManager->registerMeshBuffers();
  const auto& [vertexBuffer, indexBuffer] = resourceManager->getDebugMeshBuffers();
  commandBuffer.bindVertexBuffers(0, vertexBuffer.getBuffer(), {0});
  commandBuffer.bindIndexBuffer(indexBuffer.getBuffer(), 0, vk::IndexType::eUint32);

  commandBuffer.pushConstants<DebugPushConstants>(pipeline->getPipelineLayout(),
                                                  vk::ShaderStageFlagBits::eVertex,
                                                  0,
                                                  pushConstants);

  auto& indirectBuffer = bufferManager->getBuffer(frame->getBufferHandle2(drawCommandBufferKey));
  auto& countBuffer = bufferManager->getBuffer(frame->getBufferHandle2(objectCountBufferKey));

  commandBuffer.drawIndexedIndirectCount(indirectBuffer.getBuffer(),
                                         0,
                                         countBuffer.getBuffer(),
                                         0,
                                         renderConfig.maxDebugObjects,
                                         sizeof(vk::DrawIndexedIndirectCommand));
}

auto DebugTask::registerBuffers(const std::unique_ptr<Frame>& frame) -> void {
  // Gpu Buffer Entry Data
  {
    std::vector<GpuBufferEntry> gpuBufferEntryList{};
    gpuBufferEntryList.reserve(1);
    const auto name = frame->getIndexedName("Buffer-DebugGpuBufferEntry-Frame_");
    const auto handle = bufferManager->createBuffer(
        sizeof(GpuBufferEntry) * renderConfig.maxDebugObjects,
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        name);

    gpuBufferHandleKey = frame->registerBuffer(handle);

    auto& gpuBufferEntriesBuffer = bufferManager->getBuffer(handle);
    gpuBufferEntriesBuffer.mapBuffer();
    gpuBufferEntriesBuffer.updateBufferValue(gpuBufferEntryList.data(), sizeof(GpuBufferEntry));
    gpuBufferEntriesBuffer.unmapBuffer();
  }

  // IndirectCommandBuffer
  {
    const auto name = frame->getIndexedName("Buffer-DebugDrawCommand-Frame_");
    const auto handle = bufferManager->createBuffer(
        sizeof(vk::DrawIndexedIndirectCommand) * renderConfig.maxDebugObjects,
        vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        name);

    drawCommandBufferKey = frame->registerBuffer(handle);

    auto& indirectCommandBuffer = bufferManager->getBuffer(handle);
    auto cmd = vk::DrawIndexedIndirectCommand{};
    indirectCommandBuffer.mapBuffer();
    indirectCommandBuffer.updateBufferValue(&cmd, sizeof(vk::DrawIndexedIndirectCommand));
    indirectCommandBuffer.unmapBuffer();
  }

  // Object Count Buffer
  {
    const auto name = frame->getIndexedName("Buffer-DebugCount-Frame_");
    const auto handle = bufferManager->createBuffer(
        sizeof(uint32_t),
        vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        name);
    auto& countBuffer = bufferManager->getBuffer(handle);

    uint32_t count = 0;
    countBuffer.mapBuffer();
    countBuffer.updateBufferValue(&count, sizeof(uint32_t));
    countBuffer.unmapBuffer();

    objectCountBufferKey = frame->registerBuffer(handle);
  }

  // ObjectDataIndex Buffer
  {
    const auto name = frame->getIndexedName("Buffer-DebugObjectDataIndex-Frame_");
    const auto handle = bufferManager->createBuffer(
        sizeof(uint32_t) * renderConfig.maxDebugObjects,
        vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        name);
    auto& objectDataIndexBuffer = bufferManager->getBuffer(handle);

    uint32_t count = 0;
    objectDataIndexBuffer.mapBuffer();
    objectDataIndexBuffer.updateBufferValue(&count, sizeof(uint32_t));
    objectDataIndexBuffer.unmapBuffer();
    objectDataBufferKey = frame->registerBuffer(handle);
  }

  // Object Data Buffer
  {
    const auto name = frame->getIndexedName("Buffer-GpuObjectData-Frame_");
    const auto handle = bufferManager->createBuffer(
        sizeof(DebugGpuObjectData) * renderConfig.maxDebugObjects,
        vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        name);
    objectDataBufferKey = frame->registerBuffer(handle);
  }
}

}
