#pragma once

namespace tr {

struct GeometryData {
  std::shared_ptr<std::vector<std::byte>> indexData;
  std::shared_ptr<std::vector<std::byte>> positionData;
  std::shared_ptr<std::vector<std::byte>> colorData;
  std::shared_ptr<std::vector<std::byte>> texCoordData;
  std::shared_ptr<std::vector<std::byte>> normalData;
  std::shared_ptr<std::vector<std::byte>> animationData;

  auto getSize() -> size_t {
    return (indexData ? indexData->size() : 0L) + (positionData ? positionData->size() : 0L) +
           (colorData ? colorData->size() : 0L) + (texCoordData ? texCoordData->size() : 0L) +
           (normalData ? normalData->size() : 0L) + (animationData ? animationData->size() : 0L);
  }
};

}
