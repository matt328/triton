#pragma once

#include "vk/command-buffer/QueueType.hpp"

namespace tr {

struct PoolKey {
  std::thread::id threadId;
  uint8_t frameId;
  QueueType queueType;

  auto operator==(const PoolKey& other) const -> bool {
    return threadId == other.threadId && frameId == other.frameId && queueType == other.queueType;
  }

  auto operator<(const PoolKey& other) const -> bool {
    if (threadId != other.threadId)
      return threadId < other.threadId;
    if (frameId != other.frameId)
      return frameId < other.frameId;
    return static_cast<uint8_t>(queueType) < static_cast<uint8_t>(other.queueType);
  }
};

}

namespace std {
template <>
struct hash<tr::PoolKey> {
  auto operator()(const tr::PoolKey& key) const -> std::size_t {
    std::size_t h1 = std::hash<std::thread::id>{}(key.threadId);
    std::size_t h2 = std::hash<uint8_t>{}(key.frameId);
    std::size_t h3 = std::hash<uint8_t>{}(static_cast<uint8_t>(key.queueType));
    return h1 ^ (h2 << 1) ^ (h3 << 2); // simple mixing
  }
};
}
