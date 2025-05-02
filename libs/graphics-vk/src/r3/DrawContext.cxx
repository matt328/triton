#include <utility>

#include "DrawContext.hpp"
#include "mem/buffer-registry/BufferRegistry.hpp"
#include "task/Frame.hpp"

namespace tr {

DrawContext::DrawContext(DrawContextConfig config,
                         std::shared_ptr<BufferRegistry> newBufferRegistry)
    : bufferRegistry{std::move(newBufferRegistry)}, config{std::move(config)} {
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
    auto address = bufferRegistry->getBufferAddress(frame->getLogicalBuffer(handle));
    pcBlob.data.insert(pcBlob.data.end(), address, address + sizeof(address));
  }

  const auto pushConstantInfo = vk::PushConstantsInfo{
      .layout = layout,
      .stageFlags = pcBlob.stageFlags,
      .offset = pcBlob.offset,
      .pValues = pcBlob.data.data(),
  };
  commandBuffer.pushConstants2(pushConstantInfo);
}

auto DrawContext::record(const Frame* frame, vk::raii::CommandBuffer& commandBuffer) -> void {
  auto indirectBufferHandle = frame->getLogicalBuffer(config.indirectBuffer);
  auto* indirectBuffer = bufferRegistry->getBufferWrapper(indirectBufferHandle).get<ArenaBuffer>();

  auto* countBuffer = bufferRegistry->getBufferWrapper(frame->getLogicalBuffer(config.countBuffer))
                          .get<ArenaBuffer>();

  commandBuffer.drawIndexedIndirectCount(indirectBuffer->getBuffer().getBuffer(),
                                         config.indirectOffset,
                                         countBuffer->getBuffer().getBuffer(),
                                         config.countOffset,
                                         1024,
                                         sizeof(vk::DrawIndexedIndirectCommand));
}

}
