#pragma once

#include "IBufferManager.hpp"
#include "bk/Rando.hpp"
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

  auto getOrCreate(const BufferKey& key) -> BufferHandle;

private:
  std::shared_ptr<IBufferManager> bufferManager;

  MapKey bufferKeygen;
  std::unordered_map<BufferKey, BufferHandle> bufferHandles;

  static auto fromProfile(const BufferUsageProfile& profile) -> BufferCreateInfo;
  static auto fromProfileGeometry(const BufferUsageProfile& profile)
      -> ArenaGeometryBufferCreateInfo;
};

}
