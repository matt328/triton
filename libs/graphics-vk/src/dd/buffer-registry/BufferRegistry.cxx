#include "dd/buffer-registry/BufferRegistry.hpp"
#include "vk/ArenaGeometryBuffer.hpp"

namespace tr {

BufferRegistry::BufferRegistry(std::shared_ptr<IBufferManager> newBufferManager)
    : bufferManager{std::move(newBufferManager)} {
}

auto BufferRegistry::getOrCreateGeometryBuffer(const GeometryBufferConfig& bufferConfig)
    -> BufferHandle {
  if (geometryBufferHandles.contains(bufferConfig)) {
    return geometryBufferHandles.at(bufferConfig);
  }

  const auto createInfo = ArenaGeometryBufferCreateInfo{
      .vertexSize = static_cast<size_t>(bufferConfig.vertexFormat.stride * 102400),
      .indexSize = 102400,
      .bufferName = "Unnamed buffer"};

  const auto key = geometryBufferKeygen.getKey();
  geometryBuffers.emplace(key, std::make_unique<ArenaGeometryBuffer>(bufferManager, createInfo));

  geometryBufferHandles.emplace(bufferConfig, key);

  return key;
}

auto BufferRegistry::getOrCreateObjectBuffer(const ObjectBufferConfig& bufferConfig)
    -> BufferHandle {
  if (objectBufferHandles.contains(bufferConfig)) {
    return objectBufferHandles.at(bufferConfig);
  }
}

auto BufferRegistry::getOrCreateMaterialBuffer() -> BufferHandle {
}

}
