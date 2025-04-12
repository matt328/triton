#pragma once

#include "bk/Rando.hpp"
#include "dd/buffer-registry/BufferConfig.hpp"
#include "vk/ResourceManagerHandles.hpp"

namespace tr {

class ArenaGeometryBuffer;
class ArenaBuffer;
class IBufferManager;
class Buffer;

struct BufferUsageProfile {
  enum class Purpose : uint8_t {
    Vertex,
    Index,
    ObjectData,
    MaterialData,
    IndirectArgs
  };

  Purpose purpose;

  std::optional<VertexFormat> vertexFormat;
  std::optional<size_t> stride;

  bool perFrame = false;
  bool perDrawContext = false;
  bool mappable = false;
};

struct BufferInstanceKey {
  size_t drawContextId;
  uint8_t frameId;
};

struct BufferKey {
  BufferUsageProfile profile;
  BufferInstanceKey instance;
};

class BufferRegistry {
public:
  explicit BufferRegistry(std::shared_ptr<IBufferManager> newBufferManager);
  ~BufferRegistry() = default;

  BufferRegistry(const BufferRegistry&) = delete;
  BufferRegistry(BufferRegistry&&) = delete;
  auto operator=(const BufferRegistry&) -> BufferRegistry& = delete;
  auto operator=(BufferRegistry&&) -> BufferRegistry& = delete;

  auto getOrCreate(const BufferKey& key) -> BufferHandle;

private:
  std::shared_ptr<IBufferManager> bufferManager;

  MapKey bufferKeygen;
  std::unordered_map<BufferKey, BufferHandle> bufferHandles;
};

}
