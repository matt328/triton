#pragma once

#include "IBufferManager.hpp"
#include "bk/Handle.hpp"
#include "bk/HandleGenerator.hpp"
#include "mem/BufferWrapper.hpp"
#include "mem/buffer-registry/BufferKey.hpp"
#include "mem/buffer-registry/BufferRequest.hpp"

namespace tr {

class ArenaGeometryBuffer;
class ArenaBuffer;
class IBufferManager;
class ManagedBuffer;

class BufferRegistry {
public:
  explicit BufferRegistry(std::shared_ptr<IBufferManager> newBufferManager);
  ~BufferRegistry() = default;

  BufferRegistry(const BufferRegistry&) = delete;
  BufferRegistry(BufferRegistry&&) = delete;
  auto operator=(const BufferRegistry&) -> BufferRegistry& = delete;
  auto operator=(BufferRegistry&&) -> BufferRegistry& = delete;

  auto getOrCreate(const BufferKey& key) -> Handle<BufferWrapper>;
  auto getBufferWrapper(Handle<BufferWrapper> handle) -> BufferWrapper&;
  auto getBufferAddress(Handle<BufferWrapper> handle) -> uint64_t;

  auto registerBuffer(const BufferRequest& request) -> Handle<BufferWrapper>;
  auto registerPerFrameBuffer(const BufferRequest& request) -> LogicalHandle<BufferWrapper>;
  auto getBuffer(Handle<BufferWrapper> handle);

private:
  std::shared_ptr<IBufferManager> bufferManager;

  HandleGenerator<BufferWrapper> bufferHandleGenerator;
  std::unordered_map<BufferKey, Handle<BufferWrapper>> bufferHandles;

  static auto fromProfile(const BufferUsageProfile& profile) -> BufferCreateInfo;
  static auto fromProfileGeometry(const BufferUsageProfile& profile)
      -> ArenaGeometryBufferCreateInfo;
};

}
