#include "ForwardDrawContext.hpp"

namespace tr {
ForwardDrawContext::ForwardDrawContext(std::string newId,
                                       std::shared_ptr<BufferSystem> newBufferSystem,
                                       ForwardDrawContextCreateInfo newCreateInfo)
    : IDispatchContext{std::move(newId), std::move(newBufferSystem)}, createInfo{newCreateInfo} {
}

auto ForwardDrawContext::bind(const Frame* frame,
                              vk::raii::CommandBuffer& commandBuffer,
                              const vk::raii::PipelineLayout& layout) -> void {
}

auto ForwardDrawContext::dispatch(const Frame* frame, vk::raii::CommandBuffer& commandBuffer)
    -> void {
}
}
