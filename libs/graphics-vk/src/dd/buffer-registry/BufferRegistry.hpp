#pragma once

#include "bk/Rando.hpp"
#include "dd/buffer-registry/BufferConfig.hpp"
#include "dd/buffer-registry/BufferKey.hpp"
#include "vk/ResourceManagerHandles.hpp"

namespace tr {

class ArenaGeometryBuffer;
class ArenaBuffer;
class IBufferManager;
class Buffer;

class BufferRegistry {
public:
  explicit BufferRegistry(std::shared_ptr<IBufferManager> newBufferManager);
  ~BufferRegistry() = default;

  BufferRegistry(const BufferRegistry&) = delete;
  BufferRegistry(BufferRegistry&&) = delete;
  auto operator=(const BufferRegistry&) -> BufferRegistry& = delete;
  auto operator=(BufferRegistry&&) -> BufferRegistry& = delete;

  auto getOrCreateBuffer(const BufferConfig& bufferConfig,
                         size_t drawContextId = 0,
                         uint8_t frameId = 0) -> BufferHandle;

private:
  std::shared_ptr<IBufferManager> bufferManager;

  std::unordered_map<BufferKey, BufferHandle> geometryBufferHandles;
  std::unordered_map<BufferKey, BufferHandle> objectBufferHandles;
  std::unordered_map<BufferKey, BufferHandle> storageBufferHandles;

  MapKey geometryBufferKeygen;
  MapKey regularBufferKeygen;
  MapKey objectBufferKeygen;

  std::unordered_map<BufferHandle, std::unique_ptr<ArenaGeometryBuffer>> geometryBuffers;
  std::unordered_map<BufferHandle, std::unique_ptr<ArenaBuffer>> objectDataBuffers;
  // These are a regular 'buffer' and not a wrapper so there's just another handle
  std::unordered_map<BufferHandle, BufferHandle> regularBuffers;
};

}
