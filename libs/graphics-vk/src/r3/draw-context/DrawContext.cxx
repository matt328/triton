#include "DrawContext.hpp"
#include "buffers/BufferSystem.hpp"
#include "r3/draw-context/PushConstantBlob.hpp"
#include "task/Frame.hpp"

namespace tr {

DrawContext::DrawContext(DrawContextConfig config, std::shared_ptr<BufferSystem> newBufferSystem)
    : bufferSystem{std::move(newBufferSystem)}, config{std::move(config)} {
}

auto DrawContext::bind(const Frame* frame,
                       vk::raii::CommandBuffer& commandBuffer,
                       vk::raii::PipelineLayout& layout) -> void {
  auto pcBlob = PushConstantBlob{
      .data = {},
      .stageFlags = vk::ShaderStageFlagBits::eVertex,
      .offset = 0,
  };
  pcBlob.data.reserve(config.logicalBuffers.size() * 8);
  for (const auto& handle : config.logicalBuffers) {
    auto address = bufferSystem->getBufferAddress(frame->getLogicalBuffer(handle));
    pcBlob.data.insert(pcBlob.data.end(), address, address + sizeof(address));
  }

  const auto pushConstantInfo = vk::PushConstantsInfo{
      .layout = layout,
      .stageFlags = pcBlob.stageFlags,
      .offset = pcBlob.offset,
      .size = static_cast<uint32_t>(pcBlob.data.size() * sizeof(uint8_t)),
      .pValues = pcBlob.data.data(),
  };
  commandBuffer.pushConstants2(pushConstantInfo);
}

auto DrawContext::record(const Frame* frame, vk::raii::CommandBuffer& commandBuffer) -> void {
  auto indirectBufferHandle = frame->getLogicalBuffer(config.indirectBuffer);
  const auto indirectBuffer = bufferSystem->getVkBuffer(indirectBufferHandle);
  const auto countBuffer = bufferSystem->getVkBuffer(frame->getLogicalBuffer(config.countBuffer));

  commandBuffer.drawIndexedIndirectCount(indirectBuffer,
                                         config.indirectMetadata.indirectOffset,
                                         countBuffer,
                                         config.indirectMetadata.countOffset,
                                         1024,
                                         sizeof(vk::DrawIndexedIndirectCommand));
}

auto DrawContext::updateIndirectMetadata(const IndirectMetadata& data) -> void {
  config.indirectMetadata = data;
}

}
