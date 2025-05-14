#pragma once

namespace tr {
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
  BufferType bufferType = BufferType::DeviceArena;
  BufferUsage bufferUsage = BufferUsage::Storage;
  size_t initialSize = 1024;
  size_t itemStride = 0;
  std::string debugName;
};
}
