#include "DispatchContext.hpp"
#include "buffers/BufferSystem.hpp"
#include "r3/draw-context/PushConstantBlob.hpp"
#include "task/Frame.hpp"

namespace tr {

DispatchContext::DispatchContext(DispatchContextConfig newConfig,
                                 std::shared_ptr<BufferSystem> newBufferSystem)
    : bufferSystem{std::move(newBufferSystem)}, config{std::move(newConfig)} {
}

auto DispatchContext::bind(const Frame* frame,
                           vk::raii::CommandBuffer& commandBuffer,
                           const vk::raii::PipelineLayout& layout) -> void {
  auto pcBlob = config.pushConstantBuilder(config, *frame);

  const auto pushConstantInfo =
      vk::PushConstantsInfo{.layout = layout,
                            .stageFlags = pcBlob.stageFlags,
                            .offset = pcBlob.offset,
                            .size = static_cast<uint32_t>(pcBlob.data.size() * sizeof(uint8_t)),
                            .pValues = pcBlob.data.data()};
  commandBuffer.pushConstants2(pushConstantInfo);
}

auto DispatchContext::dispatch(const Frame* frame, vk::raii::CommandBuffer& commandBuffer) -> void {
  commandBuffer.dispatch(1024, 1, 1);
}

}
