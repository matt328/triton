#include "DispatchContext.hpp"

namespace tr {

DispatchContext::DispatchContext(DispatchContextConfig newConfig,
                                 std::shared_ptr<BufferSystem> newBufferSystem)
    : bufferSystem{std::move(newBufferSystem)}, config{std::move(newConfig)} {
}

auto DispatchContext::bind(const Frame* frame,
                           vk::raii::CommandBuffer& commandBuffer,
                           const vk::raii::PipelineLayout& layout) -> void {
}

auto DispatchContext::dispatch(const Frame* frame, vk::raii::CommandBuffer& commandBuffer) -> void {
  commandBuffer.dispatch(1024, 1, 1);
}

}
