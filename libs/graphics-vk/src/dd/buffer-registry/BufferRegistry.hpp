#pragma once

#include "bk/Rando.hpp"
#include "dd/buffer-registry/GeometryBufferConfig.hpp"
#include "dd/buffer-registry/MaterialBufferConfig.hpp"
#include "dd/buffer-registry/ObjectBufferConfig.hpp"
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

  auto getOrCreateBuffer(const GeometryBufferConfig& bufferConfig) -> BufferHandle;
  auto getOrCreateBuffer(const ObjectBufferConfig& bufferConfig) -> BufferHandle;
  auto getOrCreateBuffer(const MaterialBufferConfig& bufferConfig) -> BufferHandle;

private:
  std::shared_ptr<IBufferManager> bufferManager;

  std::unordered_map<GeometryBufferConfig, BufferHandle> geometryBufferHandles;
  std::unordered_map<ObjectBufferConfig, BufferHandle> objectBufferHandles;
  std::unordered_map<MaterialBufferConfig, BufferHandle> materialBufferHandles;

  MapKey geometryBufferKeygen;
  MapKey materialBufferKeygen;
  std::unordered_map<BufferHandle, std::unique_ptr<ArenaGeometryBuffer>> geometryBuffers;
  std::unordered_map<BufferHandle, std::unique_ptr<ArenaBuffer>> objectDataBuffers;
  std::unordered_map<BufferHandle, std::unique_ptr<Buffer>> materialBuffers;
};

}
