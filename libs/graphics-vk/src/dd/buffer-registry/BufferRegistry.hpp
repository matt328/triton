#pragma once

#include "dd/buffer-registry/GeometryBufferConfig.hpp"
#include "dd/buffer-registry/ObjectBufferConfig.hpp"
#include "vk/ResourceManagerHandles.hpp"

namespace tr {

class BufferRegistry {
public:
  BufferRegistry() = default;
  ~BufferRegistry() = default;

  BufferRegistry(const BufferRegistry&) = default;
  BufferRegistry(BufferRegistry&&) = delete;
  auto operator=(const BufferRegistry&) -> BufferRegistry& = default;
  auto operator=(BufferRegistry&&) -> BufferRegistry& = delete;

  auto getOrCreateGeometryBuffer(GeometryBufferConfig bufferConfig)
      -> std::tuple<BufferHandle, BufferHandle>;
  auto getOrCreateObjectBuffer() -> BufferHandle;
  auto getOrCreateMaterialBuffer() -> BufferHandle;

private:
  std::unordered_map<GeometryBufferConfig, std::tuple<BufferHandle, BufferHandle>> geometryBuffers;
  std::unordered_map<ObjectBufferConfig, BufferHandle> objectBuffers;
};

}
