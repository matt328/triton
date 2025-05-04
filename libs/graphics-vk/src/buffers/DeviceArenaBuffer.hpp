#pragma once

#include "IBuffer.hpp"

namespace tr {

using RegionContainer = std::set<BufferRegion, BufferRegionComparator>;

struct DeviceArenaBufferCreateInfo {
  uint32_t initialCapacity;
  uint32_t itemStride;
};

class DeviceArenaBuffer : public IBuffer {
public:
  DeviceArenaBuffer(std::shared_ptr<Device> newDevice,
                    vk::Buffer newVkBuffer,
                    std::shared_ptr<vma::Allocator> newAllocator,
                    vma::Allocation newAllocation,
                    const DeviceArenaBufferCreateInfo& createInfo);
  ~DeviceArenaBuffer() override = default;

  DeviceArenaBuffer(const DeviceArenaBuffer&) = default;
  DeviceArenaBuffer(DeviceArenaBuffer&&) = delete;
  auto operator=(const DeviceArenaBuffer&) -> DeviceArenaBuffer& = default;
  auto operator=(DeviceArenaBuffer&&) -> DeviceArenaBuffer& = delete;

  auto setData(const void* data, size_t size) -> BufferRegion override;

private:
  RegionContainer freeList;
  size_t capacity;
  size_t maxOffset{};
  size_t itemStride;

  auto mergeRegionWithNeighbors(const RegionContainer::iterator& it) -> void;
};

}
