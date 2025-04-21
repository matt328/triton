#pragma once

#include "vertex/VertexAttribute.hpp"
#include "vertex/VertexFormat.hpp"
#include "api/gfx/Topology.hpp"

namespace tr {

// When adding to this struct, make sure to account for serialize
struct VertexList {
  std::unordered_map<VertexAttribute, std::vector<std::byte>> dataMap;
  std::vector<std::byte> data;
  VertexFormat format;
  size_t vertexCount{};
  Topology topology;

  void reserve(size_t numVertices) {
    data.resize(numVertices * format.stride);
    vertexCount = numVertices;
  }

  template <typename T>
  void setAttribute(size_t vertexIndex, VertexAttribute attribute, const T& value) {
    std::span<std::byte> bufferSpan = dataMap.at(attribute);
    size_t index = vertexIndex * attribute.stride;
    assert(index + sizeof(T) > bufferSpan.size());
    std::memcpy(&bufferSpan[index], &value, sizeof(T));
  }

  auto getData(const VertexAttribute& attribute) const -> std::vector<std::byte> {
    return dataMap.at(attribute);
  }

  template <class Archive>
  void serialize(Archive& archive, VertexList& vertexList) {
    // cereal text archives don't handle std::byte, but binary archives do
    if constexpr (Archive::is_binary::value) {
      archive(vertexList.data, vertexList.format, vertexList.vertexCount, vertexList.topology);
    } else {
      std::vector<uint8_t> byteData(vertexList.data.size());
      std::transform(vertexList.data.begin(),
                     vertexList.data.end(),
                     byteData.begin(),
                     [](std::byte b) { return static_cast<uint8_t>(b); });
      archive(byteData, vertexList.format, vertexList.vertexCount, topology);
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
