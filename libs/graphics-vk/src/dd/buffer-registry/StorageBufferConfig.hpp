#pragma once

namespace tr {
struct StorageBufferConfig {
  size_t id;
  size_t size;

  auto operator==(const StorageBufferConfig& other) const -> bool {
    return id == other.id && size == other.size;
  }
};
}

namespace std {
template <>
struct hash<tr::StorageBufferConfig> {
  auto operator()(const tr::StorageBufferConfig& config) const noexcept -> std::size_t {
    return config.id | config.size;
  }
};
}
