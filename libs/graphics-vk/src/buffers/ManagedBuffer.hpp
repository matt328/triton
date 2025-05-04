#pragma once

#include "mem/BufferRegion.hpp"

namespace tr {

using RegionContainer = std::set<BufferRegion, BufferRegionComparator>;

struct ArenaInfo {
  RegionContainer freeList;
  size_t capacity;
  size_t maxOffset{};
  size_t itemStride;
};

struct BufferMeta {
  vk::BufferCreateInfo* bufferCreateInfo;
  vma::AllocationInfo allocationInfo;
  vma::AllocationCreateInfo* allocationCreateInfo;
  std::optional<ArenaInfo> arenaInfo = std::nullopt;
};

class ManagedBuffer {
public:
  ManagedBuffer(vk::Buffer newVkBuffer,
                std::shared_ptr<vma::Allocator> newAllocator,
                vma::Allocation newAllocation);
  ~ManagedBuffer();

  ManagedBuffer(const ManagedBuffer&) = delete;
  ManagedBuffer(ManagedBuffer&&) = delete;
  auto operator=(const ManagedBuffer&) -> ManagedBuffer& = delete;
  auto operator=(ManagedBuffer&&) -> ManagedBuffer& = delete;

  [[nodiscard]] auto getVkBuffer() const -> vk::Buffer&;

  auto isMappable() -> bool;

private:
  vk::Buffer vkBuffer;
  std::shared_ptr<vma::Allocator> allocator;
  vma::Allocation allocation;
};

}
