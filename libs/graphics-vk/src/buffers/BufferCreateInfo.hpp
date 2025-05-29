#pragma once

namespace tr {

enum class AllocationStrategy : uint8_t {
  None = 0,
  Linear,
  Arena
};

enum class BufferLifetime : uint8_t {
  Transient = 0,
  Persistent
};

enum class BufferType : uint8_t {
  DeviceArena = 0,
  Device,
  HostTransient,
  IndirectCommand
};

enum class BufferUsage : uint8_t {
  Storage = 0,
  Uniform,
  Transfer,
};

struct BufferCreateInfo {
  AllocationStrategy allocationStrategy = AllocationStrategy::None;
  BufferLifetime bufferLifetime;
  BufferUsage bufferUsage = BufferUsage::Storage;
  size_t initialSize = 1024;
  size_t itemStride = 0;
  std::string debugName;
  bool indirect{false};
};
}
