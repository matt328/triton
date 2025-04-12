#pragma once

#include "IBufferManager.hpp"
#include "BufferRegion.hpp"

namespace tr {

using RegionContainer = std::set<BufferRegion, BufferRegionComparator>;

class ArenaBuffer {
public:
  ArenaBuffer(IBufferManager* newBufferManager, ArenaBufferCreateInfo createInfo);
  ~ArenaBuffer();

  ArenaBuffer(const ArenaBuffer&) = delete;
  ArenaBuffer(ArenaBuffer&&) = default;
  auto operator=(const ArenaBuffer&) -> ArenaBuffer& = delete;
  auto operator=(ArenaBuffer&&) -> ArenaBuffer& = delete;

  auto insertData(const void* data, size_t size) -> BufferRegion;
  auto removeData(const BufferRegion& bufferIndex) -> void;

  [[nodiscard]] auto getBuffer() const -> Buffer&;

private:
  IBufferManager* bufferManager;

  RegionContainer freeList;

  BufferHandle bufferHandle;
  size_t capacity;
  size_t maxOffset{};
  size_t itemStride;

  auto mergeWithNeighbors(const RegionContainer::iterator& it) -> void;
};

}
