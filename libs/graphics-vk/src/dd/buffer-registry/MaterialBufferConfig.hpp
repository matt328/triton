#pragma once

namespace tr {

struct MaterialBufferConfig {
  uint8_t id;

  auto operator==(const MaterialBufferConfig& other) const -> bool {
    return id == other.id;
  }
};
}

namespace std {
template <>
struct hash<tr::MaterialBufferConfig> {
  auto operator()(const tr::MaterialBufferConfig& config) const noexcept -> std::size_t {
    return static_cast<size_t>(config.id);
  }
};
}
