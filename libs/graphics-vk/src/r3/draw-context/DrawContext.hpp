#pragma once

#include "bk/Handle.hpp"
#include "buffers/ManagedBuffer.hpp"

namespace tr {

struct PushConstantBlob {
  std::vector<uint8_t> data;
  vk::ShaderStageFlags stageFlags;
  uint32_t offset = 0;
};

struct IndirectMetadata {
  uint32_t indirectOffset;
  uint32_t countOffset;
};

struct DrawContextConfig {
  std::vector<LogicalHandle<ManagedBuffer>> logicalBuffers;
  LogicalHandle<ManagedBuffer> indirectBuffer;
  LogicalHandle<ManagedBuffer> countBuffer;
  IndirectMetadata indirectMetadata;
};

class Frame;
class BufferSystem;

class DrawContext {
public:
  DrawContext(DrawContextConfig config, std::shared_ptr<BufferSystem> newBufferSystem);
  ~DrawContext() = default;

  DrawContext(const DrawContext&) = default;
  DrawContext(DrawContext&&) = delete;
  auto operator=(const DrawContext&) -> DrawContext& = default;
  auto operator=(DrawContext&&) -> DrawContext& = delete;

  auto updateIndirectMetadata(const IndirectMetadata& data) -> void;

  auto bind(const Frame* frame,
            vk::raii::CommandBuffer& commandBuffer,
            vk::raii::PipelineLayout& layout) -> void;
  auto record(const Frame* frame, vk::raii::CommandBuffer& commandBuffer) -> void;

private:
  std::shared_ptr<BufferSystem> bufferSystem;
  DrawContextConfig config;
};

}
