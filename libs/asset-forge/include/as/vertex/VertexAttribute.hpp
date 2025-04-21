#pragma once

namespace tr {
enum class VertexAttributeFormat : uint8_t {
  Float2 = 0,
  Float3,
  Float4
};

enum class VertexAttributeSemantic : uint8_t {
  Position = 0,
  Color,
  TexCoords,
  Normal,
};

struct VertexAttribute {
  VertexAttributeFormat format;
  uint32_t stride;
  VertexAttributeSemantic semantic;

  auto operator==(const VertexAttribute& other) const -> bool {
    return format == other.format && stride == other.stride && semantic == other.semantic;
  }

  auto operator<(const VertexAttribute& other) const -> bool {
    return std::tie(format, stride, semantic) <
           std::tie(other.format, other.stride, other.semantic);
  }

  template <class Archive>
  void serialize(Archive& archive, VertexAttribute& attr) {
    archive(reinterpret_cast<uint32_t&>(attr.format),
            attr.stride,
            reinterpret_cast<uint32_t&>(attr.semantic));
  }
};

}

namespace std {
template <>
struct hash<tr::VertexAttribute> {
  auto operator()(const tr::VertexAttribute& attr) const noexcept -> size_t {
    size_t h1 = hash<int>()(static_cast<int>(attr.format));
    size_t h2 = hash<uint32_t>()(attr.stride);
    size_t h3 = hash<int>()(static_cast<int>(attr.semantic));
    return h1 ^ (h2 << 1U) ^ (h3 << 2U); // Combine hashes
  }
};
}
