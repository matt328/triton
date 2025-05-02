#pragma once

#include "IBufferManager.hpp"
#include "bk/Handle.hpp"
#include "bk/Rando.hpp"
#include "mem/BufferWrapper.hpp"
#include "mem/buffer-registry/BufferKey.hpp"
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
  auto getBufferWrapper(Handle<BufferWrapper> handle) -> BufferWrapper&;
  auto getBufferAddress(Handle<BufferWrapper> handle) -> uint64_t;

private:
  std::shared_ptr<IBufferManager> bufferManager;

  MapKey bufferKeygen;
  std::unordered_map<BufferKey, BufferHandle> bufferHandles;

  static auto fromProfile(const BufferUsageProfile& profile) -> BufferCreateInfo;
  static auto fromProfileGeometry(const BufferUsageProfile& profile)
      -> ArenaGeometryBufferCreateInfo;
};

}
