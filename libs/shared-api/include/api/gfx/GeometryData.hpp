#pragma once

namespace tr {

struct GeometryData {
  std::shared_ptr<std::vector<std::byte>> indexData;
  std::shared_ptr<std::vector<std::byte>> positionData;
  std::shared_ptr<std::vector<std::byte>> colorData;
  std::shared_ptr<std::vector<std::byte>> texCoordData;
  std::shared_ptr<std::vector<std::byte>> normalData;
  std::shared_ptr<std::vector<std::byte>> animationData;
};

}
