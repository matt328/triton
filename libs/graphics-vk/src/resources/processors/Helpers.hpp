#pragma once

#include "api/gfx/GeometryData.hpp"
#include "api/gfx/GpuMaterialData.hpp"
#include "as/Model.hpp"
#include "as/StaticVertex.hpp"
#include "resources/ByteConverters.hpp"

namespace tr::processorHelpers {

/// Eventually Update the TRM model formats to store data on disk in a deinterleaved format so
/// this method is unnecessary, but just convert it here for now.
constexpr auto deInterleave(const std::vector<as::StaticVertex>& vertices,
                            const std::vector<uint32_t>& indexData)
    -> std::shared_ptr<GeometryData> {
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

  return std::make_shared<GeometryData>(GeometryData{.indexData = indicesBytes,
                                                     .positionData = positionBytes,
                                                     .colorData = nullptr,
                                                     .texCoordData = texCoordBytes,
                                                     .normalData = nullptr,
                                                     .animationData = nullptr});
}

inline auto toByteVector(const std::shared_ptr<as::ImageData>& imageData)
    -> std::vector<std::byte> {
  auto data = std::vector<std::byte>{};
  data.resize(imageData->data.size());
  std::ranges::transform(imageData->data, data.begin(), [](unsigned char c) {
    return static_cast<std::byte>(c);
  });
  return data;
}

inline auto getVkFormat(int bits, int component) -> vk::Format {
  if (bits == 8) {
    switch (component) {
      case 1:
        return vk::Format::eR8Unorm;
      case 2:
        return vk::Format::eR8G8Unorm;
      case 3:
        return vk::Format::eR8G8B8Unorm;
      case 4:
        return vk::Format::eR8G8B8A8Unorm;
      default:
        return vk::Format::eR8Unorm;
    }
  } else if (bits == 16) {
    switch (component) {
      case 1:
        return vk::Format::eR16Unorm;
      case 2:
        return vk::Format::eR16G16Unorm;
      case 3:
        return vk::Format::eR16G16B16Unorm;
      case 4:
        return vk::Format::eR16G16B16A16Unorm;
      default:
        return vk::Format::eR16Unorm;
    }
  } else if (bits == 32) {
    switch (component) {
      case 1:
        return vk::Format::eR32Sfloat;
      case 2:
        return vk::Format::eR32G32Sfloat;
      case 3:
        return vk::Format::eR32G32B32Sfloat;
      case 4:
        return vk::Format::eR32G32B32A32Sfloat;
      default:
        return vk::Format::eR32Sfloat;
    }
  }

  throw std::runtime_error("Unsupported image format: component=" + std::to_string(component) +
                           ", bits=" + std::to_string(bits));
}

}
