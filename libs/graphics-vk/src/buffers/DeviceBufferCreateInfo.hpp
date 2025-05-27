#pragma once

namespace tr {

enum class AllocationType : uint8_t {
  Linear = 0,
  Arena,
};

struct DeviceBufferCreateInfo {
  AllocationType allocationType;
  size_t initialSize;
  std::string debugName;
};
}
