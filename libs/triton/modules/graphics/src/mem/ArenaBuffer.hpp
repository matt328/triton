#pragma once

#include "IBufferManager.hpp"
#include "BufferRegion.hpp"

namespace tr {

using RegionContainer = std::set<BufferRegion, BufferRegionComparator>;

class ArenaBuffer {
public:
  ArenaBuffer(std::shared_ptr<IBufferManager> newBufferManager,
              size_t newItemStride,
              size_t initialBufferSize,
              std::string_view bufferName);
  ~ArenaBuffer();

  ArenaBuffer(const ArenaBuffer&) = delete;
  ArenaBuffer(ArenaBuffer&&) = delete;
  auto operator=(const ArenaBuffer&) -> ArenaBuffer& = delete;
  auto operator=(ArenaBuffer&&) -> ArenaBuffer& = delete;

  auto insertData(void* data, size_t size) -> BufferRegion;
  auto removeData(const BufferRegion& bufferIndex) -> void;

  [[nodiscard]] auto getBuffer() const -> Buffer&;

private:
  std::shared_ptr<IBufferManager> bufferManager;

  RegionContainer freeList;

  BufferHandle bufferHandle;
  size_t capacity;
  size_t maxOffset{};
  size_t itemStride;

  auto findEmptyRegion(size_t requiredSize) -> std::optional<std::reference_wrapper<BufferRegion>>;
  auto mergeWithNeighbors(RegionContainer::iterator it) -> void;
};

}
