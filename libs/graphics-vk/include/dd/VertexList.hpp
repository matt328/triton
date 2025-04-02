#pragma once

namespace tr {

struct VertexAttribute {
  vk::Format format;
  uint32_t offset;

  auto operator==(const VertexAttribute& other) const -> bool {
    return format == other.format && offset == other.offset;
  }

  auto operator<(const VertexAttribute& other) const -> bool {
    return std::tie(format, offset) < std::tie(other.format, other.offset);
  }

  template <class Archive>
  void serialize(Archive& archive, VertexAttribute& attr) {
    archive(reinterpret_cast<uint32_t&>(attr.format), attr.offset);
  }
};

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

struct VertexList {
  std::vector<std::byte> data;
  VertexFormat format;
  size_t vertexCount{};

  void reserve(size_t numVertices) {
    data.resize(numVertices * format.stride);
    vertexCount = numVertices;
  }

  template <typename T>
  void setAttribute(size_t vertexIndex, uint32_t attributeOffset, const T& value) {
    std::span<std::byte> bufferSpan = data;
    size_t index = (vertexIndex * format.stride) + attributeOffset;
    assert(index + sizeof(T) > bufferSpan.size());
    std::memcpy(&bufferSpan[index], &value, sizeof(T));
  }

  template <class Archive>
  void serialize(Archive& archive, VertexList& vertexList) {
    // cereal text archives don't handle std::byte, but binary archives do
    if constexpr (Archive::is_binary::value) {
      archive(vertexList.data, vertexList.format, vertexList.vertexCount);
    } else {
      std::vector<uint8_t> byteData(vertexList.data.size());
      std::transform(vertexList.data.begin(),
                     vertexList.data.end(),
                     byteData.begin(),
                     [](std::byte b) { return static_cast<uint8_t>(b); });
      archive(byteData, vertexList.format, vertexList.vertexCount);
      if constexpr (!Archive::is_saving::value) {
        vertexList.data.resize(byteData.size());
        std::transform(byteData.begin(), byteData.end(), vertexList.data.begin(), [](uint8_t b) {
          return static_cast<std::byte>(b);
        });
      }
    }
  }
};

}

namespace std {
template <>
struct hash<tr::VertexAttribute> {
  auto operator()(const tr::VertexAttribute& attr) const noexcept -> size_t {
    size_t h1 = hash<int>()(static_cast<int>(attr.format));
    size_t h2 = hash<uint32_t>()(attr.offset);
    return h1 ^ (h2 << 1U); // Combine hashes
  }
};

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
