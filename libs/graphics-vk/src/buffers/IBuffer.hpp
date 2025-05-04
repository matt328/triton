#pragma once

#include "mem/BufferRegion.hpp"

namespace tr {

struct BufferMeta {
  vk::BufferCreateInfo* bufferCreateInfo;
  vma::AllocationInfo allocationInfo;
  vma::AllocationCreateInfo* allocationCreateInfo;
};

class Device;

class IBuffer {
public:
  IBuffer(std::shared_ptr<Device> newDevice,
          vk::Buffer newVkBuffer,
          BufferMeta bufferMeta,
          std::shared_ptr<vma::Allocator> newAllocator,
          vma::Allocation newAllocation);
  virtual ~IBuffer() = default;

  IBuffer(const IBuffer&) = default;
  IBuffer(IBuffer&&) = delete;
  auto operator=(const IBuffer&) -> IBuffer& = default;
  auto operator=(IBuffer&&) -> IBuffer& = delete;

  [[nodiscard]] auto getVkBuffer() const -> const vk::Buffer&;
  [[nodiscard]] auto getAddress() const -> uint64_t;

  virtual auto setData(const void* data, size_t size) -> BufferRegion = 0;

protected:
  std::shared_ptr<Device> device;
  vk::Buffer vkBuffer;
  BufferMeta bufferMeta;
  std::shared_ptr<vma::Allocator> allocator;
  vma::Allocation allocation;
};

}

/*
  Types of buffers so far:
  - GeometryEntry Buffer - Global, Storage Buffer, GPU Only, Arena
    - VertexAttribute Buffers - Global, Storage, GPU Only, Arena
  - Material Buffer - Global, Storage buffer, GPU Only, Arena
  - ObjectData Buffer - Per Frame, Storage Buffer, Host Visible, Resizable-Non-Arena
  - DrawCommandMeta Buffer - Per Frame, Storage Buffer, Host Visible, Resizable-Non-Arena
  - DIIC - Per Frame, Storage Buffer, GPU Only, Resizable-Non-Arena
  - DrawCount - Per Frame, Storage, GPU Only, Resizable-Non-Arena
  - Camera/Other Data - Per Frame, Uniform, Host Visible, Resizable-Non-Arena

  Buffer Classes
    DeviceArena
    HostTransient


*/
