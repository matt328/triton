#pragma once

#include "IBuffer.hpp"

namespace tr {

class HostTransientBuffer : public IBuffer {
public:
  HostTransientBuffer() = default;
  ~HostTransientBuffer() override = default;

  HostTransientBuffer(const HostTransientBuffer&) = default;
  HostTransientBuffer(HostTransientBuffer&&) = delete;
  auto operator=(const HostTransientBuffer&) -> HostTransientBuffer& = default;
  auto operator=(HostTransientBuffer&&) -> HostTransientBuffer& = delete;

  [[nodiscard]] auto getVkBuffer() const -> vk::Buffer& override;
  [[nodiscard]] auto getAddress() const -> uint64_t override;

  auto setData(const void* data, size_t size) -> BufferRegion override;
};

}
