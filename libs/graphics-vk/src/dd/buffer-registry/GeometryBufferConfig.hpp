#pragma once

#include "as/VertexList.hpp"

namespace tr {
struct GeometryBufferConfig {
  VertexFormat vertexFormat;

  auto operator==(const GeometryBufferConfig& other) const -> bool {
    return vertexFormat == other.vertexFormat;
  }
};
}

namespace std {
template <>
struct hash<tr::GeometryBufferConfig> {
  auto operator()(const tr::GeometryBufferConfig& config) const noexcept -> std::size_t {
    return std::hash<tr::VertexFormat>{}(config.vertexFormat);
  }
};
}
