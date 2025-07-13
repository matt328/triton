#pragma once

#include "api/gfx/GeometryData.hpp"
#include "api/gfx/GpuMaterialData.hpp"
#include "as/StaticVertex.hpp"
#include "resources/ByteConverters.hpp"

namespace tr::processorHelpers {

/// Eventually Update the TRM model formats to store data on disk in a deinterleaved format so
/// this method is unnecessary, but just convert it here for now.
constexpr auto deInterleave(const std::vector<as::StaticVertex>& vertices,
                            const std::vector<uint32_t>& indexData)
    -> std::unique_ptr<GeometryData> {
  auto positions = std::make_shared<std::vector<GpuVertexPositionData>>();
  auto texCoords = std::make_shared<std::vector<GpuVertexTexCoordData>>();
  auto colors = std::make_shared<std::vector<GpuVertexColorData>>();
  auto indices = std::make_shared<std::vector<GpuIndexData>>();

  positions->reserve(vertices.size() * sizeof(GpuVertexPositionData));
  texCoords->reserve(vertices.size() * sizeof(GpuVertexTexCoordData));
  colors->reserve(vertices.size() * sizeof(GpuVertexColorData));
  indices->reserve(indexData.size() * sizeof(GpuIndexData));

  for (const auto& vertex : vertices) {
    positions->emplace_back(vertex.position);
    texCoords->emplace_back(vertex.texCoord);
  }

  for (auto index : indexData) {
    indices->emplace_back(index);
  }

  auto indicesBytes = toByteVector(indices);
  auto texCoordBytes = toByteVector(texCoords);
  auto positionBytes = toByteVector(positions);

  return std::make_unique<GeometryData>(GeometryData{.indexData = indicesBytes,
                                                     .positionData = positionBytes,
                                                     .colorData = nullptr,
                                                     .texCoordData = texCoordBytes,
                                                     .normalData = nullptr,
                                                     .animationData = nullptr});
}

}
