#include "dd/buffer-registry/BufferRegistry.hpp"
#include "dd/gpu-data/GpuMaterialData.hpp"
#include "vk/ArenaGeometryBuffer.hpp"

namespace tr {

BufferRegistry::BufferRegistry(std::shared_ptr<IBufferManager> newBufferManager)
    : bufferManager{std::move(newBufferManager)} {
}

auto BufferRegistry::getOrCreateBuffer(const GeometryBufferConfig& bufferConfig) -> BufferHandle {
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

auto BufferRegistry::getOrCreateBuffer(const ObjectBufferConfig& bufferConfig) -> BufferHandle {
  if (objectBufferHandles.contains(bufferConfig)) {
    return objectBufferHandles.at(bufferConfig);
  }
  return static_cast<BufferHandle>(0L);
}

auto BufferRegistry::getOrCreateBuffer(const MaterialBufferConfig& config) -> BufferHandle {
  if (materialBufferHandles.contains(config)) {
    return materialBufferHandles.at(config);
  }
  const auto createInfo = BufferCreateInfo{
      .size = sizeof(GpuMaterialData) * 128,
      .flags = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
      .memoryUsage = vma::MemoryUsage::eCpuToGpu,
      .memoryProperties =
          vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
  };

  const auto key = materialBufferKeygen.getKey();
  materialBuffers.emplace(key, bufferManager->createBuffer(createInfo));
  materialBufferHandles.emplace(config, key);
  return key;
}

}
