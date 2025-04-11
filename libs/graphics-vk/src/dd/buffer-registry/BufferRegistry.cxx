#include "dd/buffer-registry/BufferRegistry.hpp"
#include "dd/buffer-registry/BufferKey.hpp"
#include "dd/gpu-data/GpuMaterialData.hpp"
#include "vk/ArenaGeometryBuffer.hpp"

namespace tr {

BufferRegistry::BufferRegistry(std::shared_ptr<IBufferManager> newBufferManager)
    : bufferManager{std::move(newBufferManager)} {
}

auto BufferRegistry::getOrCreateBuffer(const BufferConfig& bufferConfig,
                                       size_t drawContextId,
                                       uint8_t frameId) -> BufferHandle {
  const auto bufferKey =
      BufferKey{.config = bufferConfig, .drawContextId = drawContextId, .frameId = frameId};

  switch (bufferConfig.bufferType) {
    case BufferType::Geometry: {
      if (geometryBufferHandles.contains(bufferKey)) {
        return geometryBufferHandles.at(bufferKey);
      }
      const auto createInfo = ArenaGeometryBufferCreateInfo{
          .vertexSize = static_cast<size_t>(bufferConfig.vertexFormat->stride * 102400),
          .indexSize = 102400,
          .bufferName = "Unnamed buffer"};

      const auto handle = geometryBufferKeygen.getKey();
      geometryBuffers.emplace(handle,
                              std::make_unique<ArenaGeometryBuffer>(bufferManager, createInfo));

      geometryBufferHandles.emplace(bufferKey, handle);

      return handle;
      break;
    }
    case BufferType::Arena: {
      break;
    }
    case BufferType::Fixed: {
      break;
    }
  }
}

auto BufferRegistry::getOrCreateBuffer(const GeometryRegionBufferConfig& bufferConfig,
                                       uint32_t drawContextId,
                                       uint8_t frameId) -> BufferHandle {
  const auto bufferKey = BufferKey<GeometryRegionBufferConfig>{
      .config = bufferConfig,
      .instance = {.drawContextId = drawContextId, .frameId = frameId}};

  if (geometryRegionBufferHandles.contains(bufferKey)) {
    return geometryRegionBufferHandles.at(bufferKey);
  }

  const auto createInfo = BufferCreateInfo{
      .size = sizeof(GeometryRegion) * 128,
      .flags = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
      .memoryUsage = vma::MemoryUsage::eCpuToGpu,
      .memoryProperties =
          vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
  };
  const auto key = regularBufferKeygen.getKey();
  regularBuffers.emplace(key, bufferManager->createBuffer(createInfo));
  geometryRegionBufferHandles.emplace(bufferConfig, key);
  return key;
}

auto BufferRegistry::getOrCreateBuffer(const GeometryBufferConfig& bufferConfig,
                                       uint32_t drawContextId,
                                       uint8_t frameId) -> BufferHandle {
  const auto key = BufferKey<GeometryBufferConfig>{
      .config = bufferConfig,
      .instance = {.drawContextId = drawContextId, .frameId = frameId}};

  if (geometryBufferHandles.contains(key)) {
    return geometryBufferHandles.at(key);
  }

  const auto createInfo = ArenaGeometryBufferCreateInfo{
      .vertexSize = static_cast<size_t>(bufferConfig.vertexFormat.stride * 102400),
      .indexSize = 102400,
      .bufferName = "Unnamed buffer"};

  const auto handle = geometryBufferKeygen.getKey();
  geometryBuffers.emplace(handle, std::make_unique<ArenaGeometryBuffer>(bufferManager, createInfo));

  geometryBufferHandles.emplace(key, handle);

  return handle;
}

auto BufferRegistry::getOrCreateBuffer(const ObjectBufferConfig& bufferConfig,
                                       size_t drawContextId,
                                       uint8_t frameId) -> BufferHandle {
  const auto key = BufferKey<ObjectBufferConfig>{
      .config = bufferConfig,
      .instance = {.drawContextId = drawContextId, .frameId = frameId}};

  if (objectBufferHandles.contains(key)) {
    return objectBufferHandles.at(key);
  }

  const auto handle = objectBufferKeygen.getKey();

  const auto bci = BufferCreateInfo{

  };

  const auto actualBufferHandle = bufferManager->createBuffer(bci);

  objectDataBuffers.emplace(handle, actualBufferHandle);

  objectBufferHandles.emplace(key, handle);
  return static_cast<BufferHandle>(0L);
}

auto BufferRegistry::getOrCreateBuffer(const StorageBufferConfig& config,
                                       size_t drawContextId,
                                       uint8_t frameId) -> BufferHandle {
  const auto bufferKey = BufferKey<StorageBufferConfig>{
      .config = config,
      .instance = {.drawContextId = drawContextId, .frameId = frameId}};

  if (storageBufferHandles.contains(bufferKey)) {
    return storageBufferHandles.at(bufferKey);
  }

  const auto createInfo = BufferCreateInfo{
      .size = config.size,
      .flags = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
      .memoryUsage = vma::MemoryUsage::eCpuToGpu,
      .memoryProperties =
          vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
  };

  const auto key = regularBufferKeygen.getKey();
  regularBuffers.emplace(key, bufferManager->createBuffer(createInfo));
  storageBufferHandles.emplace(config, key);
  return key;
}

}
