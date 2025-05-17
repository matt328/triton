#include <utility>

#include "CullingDispatchContext.hpp"
#include "buffers/BufferSystem.hpp"
#include "task/Frame.hpp"

namespace tr {

constexpr uint32_t WorkgroupSize = 64;

CullingDispatchContext::CullingDispatchContext(std::string newId,
                                               std::shared_ptr<BufferSystem> newBufferSystem,
                                               CullingDispatchContextCreateInfo newCreateInfo)
    : IDispatchContext{std::move(newId), std::move(newBufferSystem)}, createInfo{newCreateInfo} {
}

auto CullingDispatchContext::bind(const Frame* frame,
                                  vk::raii::CommandBuffer& commandBuffer,
                                  const vk::raii::PipelineLayout& layout) -> void {
  const auto pushConstants = PushConstants{
      .objectCount = frame->getObjectCount(),
      .objectDataAddress =
          bufferSystem->getBufferAddress(frame->getLogicalBuffer(createInfo.objectData)),
      .outputIndirectCommandAddress =
          bufferSystem->getBufferAddress(frame->getLogicalBuffer(createInfo.indirectCommand)),
      .outputIndirectCountAddress =
          bufferSystem->getBufferAddress(frame->getLogicalBuffer(createInfo.indirectCount)),
      .geometryRegionAddress = bufferSystem->getBufferAddress(createInfo.geometryRegion),
      .indexDataAddress = bufferSystem->getBufferAddress(createInfo.indexData),
      .vertexPositionAddress = bufferSystem->getBufferAddress(createInfo.vertexPosition),
      .vertexNormalAddress = bufferSystem->getBufferAddress(createInfo.vertexNormal),
      .vertexTexCoordAddress = bufferSystem->getBufferAddress(createInfo.vertexTexCoord),
      .vertexColorAddress = bufferSystem->getBufferAddress(createInfo.vertexColor),
  };
  commandBuffer.pushConstants<PushConstants>(layout,
                                             vk::ShaderStageFlagBits::eCompute,
                                             0,
                                             pushConstants);
}

auto CullingDispatchContext::dispatch(const Frame* frame, vk::raii::CommandBuffer& commandBuffer)
    -> void {
  uint32_t workgroupCount = (frame->getObjectCount() + WorkgroupSize - 1) / WorkgroupSize;
  commandBuffer.dispatch(workgroupCount, 1, 1);
}

}
