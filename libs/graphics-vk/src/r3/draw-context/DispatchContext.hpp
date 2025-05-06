#pragma once

#include "bk/Handle.hpp"
#include "buffers/ManagedBuffer.hpp"

namespace tr {

struct DispatchContextConfig {
  std::vector<LogicalHandle<ManagedBuffer>> logicalBuffers;
};

class Frame;
class BufferSystem;

class DispatchContext {
public:
  DispatchContext(DispatchContextConfig newConfig, std::shared_ptr<BufferSystem> newBufferSystem);
  ~DispatchContext() = default;

  DispatchContext(const DispatchContext&) = default;
  DispatchContext(DispatchContext&&) = delete;
  auto operator=(const DispatchContext&) -> DispatchContext& = default;
  auto operator=(DispatchContext&&) -> DispatchContext& = delete;

  auto bind(const Frame* frame,
            vk::raii::CommandBuffer& commandBuffer,
            const vk::raii::PipelineLayout& layout) -> void;
  auto dispatch(const Frame* frame, vk::raii::CommandBuffer& commandBuffer) -> void;

private:
  std::shared_ptr<BufferSystem> bufferSystem;
  DispatchContextConfig config;
};

}
