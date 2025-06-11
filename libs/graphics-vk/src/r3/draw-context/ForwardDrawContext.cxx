#include "ForwardDrawContext.hpp"

namespace tr {
ForwardDrawContext::ForwardDrawContext(ContextId newId,
                                       std::shared_ptr<BufferSystem> newBufferSystem,
                                       ForwardDrawContextCreateInfo newCreateInfo)
    : IDispatchContext{newId, std::move(newBufferSystem)}, createInfo{newCreateInfo} {
}

auto ForwardDrawContext::bind([[maybe_unused]] const Frame* frame,
                              [[maybe_unused]] vk::raii::CommandBuffer& commandBuffer,
                              [[maybe_unused]] const vk::raii::PipelineLayout& layout) -> void {
}

auto ForwardDrawContext::dispatch([[maybe_unused]] const Frame* frame,
                                  [[maybe_unused]] vk::raii::CommandBuffer& commandBuffer) -> void {
}

auto ForwardDrawContext::getPushConstantSize() -> size_t {
  return sizeof(0);
}

[[nodiscard]] auto ForwardDrawContext::getGraphInfo() const -> PassGraphInfo {
  auto pgInfo = PassGraphInfo{};

  return pgInfo;
}
}
