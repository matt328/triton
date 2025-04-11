#pragma once

#include "dd/buffer-registry/BufferConfig.hpp"

namespace tr {
struct BufferKey {
  BufferConfig config{};
  size_t drawContextId{};
  uint32_t frameId{};

  auto operator==(const BufferKey& other) const -> bool {
    return config == other.config && drawContextId == other.drawContextId &&
           frameId == other.frameId;
  }
};
}

namespace std {
template <>
struct hash<tr::BufferKey> {
  auto operator()(const tr::BufferKey& key) const -> std::size_t {
    std::size_t h = std::hash<tr::BufferConfig>{}(key.config);
    h ^= std::hash<size_t>{}(key.drawContextId) + 0x9e3779b9 + (h << 6) + (h >> 2);
    h ^= std::hash<uint32_t>{}(key.frameId) + 0x9e3779b9 + (h << 6) + (h >> 2);
    return h;
  }
};
}
