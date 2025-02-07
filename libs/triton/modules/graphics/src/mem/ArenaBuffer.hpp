#pragma once

#include "IBufferManager.hpp"
#include "BufferRegion.hpp"

namespace tr {

using RegionContainer = std::set<BufferRegion, BufferRegionComparator>;

enum class BufferType : uint8_t {
  Vertex = 0,
  Index
};

struct ArenaBufferCreateInfo {
  size_t newItemStride;
  size_t initialBufferSize;
  BufferType bufferType;
  std::string_view bufferName;
};

class ArenaBuffer {
public:
  ArenaBuffer(std::shared_ptr<IBufferManager> newBufferManager, ArenaBufferCreateInfo createInfo);
  ~ArenaBuffer();

  ArenaBuffer(const ArenaBuffer&) = delete;
  ArenaBuffer(ArenaBuffer&&) = delete;
  auto operator=(const ArenaBuffer&) -> ArenaBuffer& = delete;
  auto operator=(ArenaBuffer&&) -> ArenaBuffer& = delete;

  auto insertData(const void* data, size_t size) -> BufferRegion;
  auto removeData(const BufferRegion& bufferIndex) -> void;

  [[nodiscard]] auto getBuffer() const -> Buffer&;

private:
  std::shared_ptr<IBufferManager> bufferManager;

  RegionContainer freeList;

  BufferHandle bufferHandle;
  size_t capacity;
  size_t maxOffset{};
  size_t itemStride;

  auto mergeWithNeighbors(const RegionContainer::iterator& it) -> void;
};

}
