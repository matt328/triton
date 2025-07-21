#pragma once

#include "bk/Handle.hpp"
#include "mem/BufferRegion.hpp"

namespace tr {

class IBufferStrategy;

using RegionContainer = std::set<BufferRegion, BufferRegionComparator>;

struct ArenaInfo {
  RegionContainer freeList;
  size_t capacity;
  size_t maxOffset{};
  size_t itemStride;
};

struct BufferMeta {
  vk::BufferCreateInfo bufferCreateInfo;
  vma::AllocationInfo allocationInfo;
  vma::AllocationCreateInfo allocationCreateInfo;
  std::string debugName;
};

class ManagedBuffer {
public:
  ManagedBuffer(vk::Buffer newVkBuffer,
                BufferMeta newBufferMeta,
                std::shared_ptr<vma::Allocator> newAllocator,
                vma::Allocation newAllocation);
  ~ManagedBuffer();

  ManagedBuffer(const ManagedBuffer&) = delete;
  ManagedBuffer(ManagedBuffer&&) = delete;
  auto operator=(const ManagedBuffer&) -> ManagedBuffer& = delete;
  auto operator=(ManagedBuffer&&) -> ManagedBuffer& = delete;

  [[nodiscard]] auto getVkBuffer() const -> const vk::Buffer&;

  auto isMappable() -> bool;

  auto map() -> void;

  auto uploadData(void* srcData, size_t size, size_t offset = 0) -> void;

  [[nodiscard]] auto getMeta() const -> const BufferMeta&;

  [[nodiscard]] auto getValidFromFrame() const -> uint64_t;
  [[nodiscard]] auto getValidToFrame() const -> std::optional<uint64_t>;

  auto setValidFromFrame(uint64_t frame) -> void;
  auto setValidToFrame(uint64_t frame) -> void;

private:
  vk::Buffer vkBuffer;
  BufferMeta bufferMeta;
  vma::Allocation allocation;
  std::shared_ptr<vma::Allocator> allocator;
  void* mappedData = nullptr;

  uint64_t validFromFrame{};
  std::optional<uint64_t> validToFrame;
};

}
