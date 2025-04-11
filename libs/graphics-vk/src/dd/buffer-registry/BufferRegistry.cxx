#include "dd/buffer-registry/BufferRegistry.hpp"
#include "dd/buffer-registry/BufferKey.hpp"
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
      Log.warn("Can't currently register ArenaBuffers");
      return static_cast<BufferHandle>(0L);
      break;
    }
    case BufferType::Fixed: {
      if (storageBufferHandles.contains(bufferKey)) {
        return storageBufferHandles.at(bufferKey);
      }
      const auto createInfo = BufferCreateInfo{
          .size = bufferConfig.size,
          .flags = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
          .memoryUsage = vma::MemoryUsage::eCpuToGpu,
          .memoryProperties =
              vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible,
      };
      const auto key = regularBufferKeygen.getKey();
      regularBuffers.emplace(key, bufferManager->createBuffer(createInfo));
      storageBufferHandles.emplace(bufferConfig, key);
      return key;
      break;
    }
  }
}

}
