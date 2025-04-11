#pragma once

namespace tr {

struct GeometryRegionBufferConfig {
  size_t id;

  auto operator==(const GeometryRegionBufferConfig& other) const -> bool {
    return id == other.id;
  }
};

}

namespace std {
template <>
struct hash<tr::GeometryRegionBufferConfig> {
  auto operator()(const tr::GeometryRegionBufferConfig& config) const noexcept -> std::size_t {
    return static_cast<size_t>(config.id);
  }
};
}
