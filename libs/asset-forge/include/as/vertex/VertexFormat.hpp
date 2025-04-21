#pragma once

#include "VertexAttribute.hpp"

namespace tr {
struct VertexFormat {
  uint32_t stride;
  std::vector<VertexAttribute> attributes;

  auto operator==(const VertexFormat& other) const -> bool {
    return stride == other.stride && attributes == other.attributes;
  }

  auto operator<(const VertexFormat& other) const -> bool {
    return std::tie(stride, attributes) < std::tie(other.stride, other.attributes);
  }
  template <class Archive>
  void serialize(Archive& archive, VertexFormat& format) {
    archive(format.stride, "attributes", format.attributes);
  }
};
}

namespace std {

template <>
struct hash<tr::VertexFormat> {
  auto operator()(const tr::VertexFormat& format) const noexcept -> size_t {
    size_t h = hash<uint32_t>()(format.stride);
    for (const auto& attr : format.attributes) {
      h ^= hash<tr::VertexAttribute>()(attr) + 0x9e3779b9 + (h << 6U) + (h >> 2U); // Hash mix
    }
    return h;
  }
};
}
