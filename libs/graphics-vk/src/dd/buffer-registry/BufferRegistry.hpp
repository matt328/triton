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

struct BufferInstanceKey {
  size_t drawContextId;
  uint32_t frameId;

  auto operator==(const BufferInstanceKey& other) const -> bool {
    return drawContextId == other.drawContextId && frameId == other.frameId;
  }
};

template <typename T>
struct BufferKey {
  T config;
  BufferInstanceKey instance;

  auto operator==(const BufferKey& other) const -> bool {
    return config == other.config && instance == other.instance;
  }
};

class BufferRegistry {
public:
  explicit BufferRegistry(std::shared_ptr<IBufferManager> newBufferManager);
  ~BufferRegistry() = default;

  BufferRegistry(const BufferRegistry&) = delete;
  BufferRegistry(BufferRegistry&&) = delete;
  auto operator=(const BufferRegistry&) -> BufferRegistry& = delete;
  auto operator=(BufferRegistry&&) -> BufferRegistry& = delete;

  auto getOrCreateBuffer(const GeometryBufferConfig& bufferConfig,
                         uint32_t drawContextId = 0,
                         uint8_t frameId = 0) -> BufferHandle;

  auto getOrCreateBuffer(const ObjectBufferConfig& bufferConfig,
                         size_t drawContextId = 0,
                         uint8_t frameId = 0) -> BufferHandle;
  auto getOrCreateBuffer(const MaterialBufferConfig& bufferConfig) -> BufferHandle;

private:
  std::shared_ptr<IBufferManager> bufferManager;

  std::unordered_map<BufferKey<GeometryBufferConfig>, BufferHandle> geometryBufferHandles;
  std::unordered_map<ObjectBufferConfig, BufferHandle> objectBufferHandles;
  std::unordered_map<MaterialBufferConfig, BufferHandle> materialBufferHandles;

  MapKey geometryBufferKeygen;
  MapKey materialBufferKeygen;
  std::unordered_map<BufferHandle, std::unique_ptr<ArenaGeometryBuffer>> geometryBuffers;
  std::unordered_map<BufferHandle, std::unique_ptr<ArenaBuffer>> objectDataBuffers;
  std::unordered_map<BufferHandle, std::unique_ptr<Buffer>> materialBuffers;
};

}

namespace std {
template <>
struct hash<tr::BufferInstanceKey> {
  auto operator()(const tr::BufferInstanceKey& key) const -> size_t {
    return std::hash<size_t>{}(key.drawContextId) ^ (std::hash<uint32_t>{}(key.frameId) << 1U);
  }
};

template <typename T>
struct hash<tr::BufferKey<T>> {
  auto operator()(const tr::BufferKey<T>& key) const -> size_t {
    return std::hash<tr::GeometryBufferConfig>{}(key.config) ^
           std::hash<tr::BufferInstanceKey>{}(key.instance);
  }
};
}
