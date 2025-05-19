#pragma once

#include "api/fx/Events.hpp"
#include "api/gfx/GpuMaterialData.hpp"
namespace tr {

struct StaticModelTask {
  std::string filename;
};

using AssetRequest = std::variant<StaticModelRequest>;

using AssetTask = std::variant<StaticModelTask>;

template <typename>
inline constexpr bool always_false = false;

struct GeometryData {
  std::vector<GpuIndexData> indexData;
  std::vector<GpuVertexPositionData> positionData;
  std::vector<GpuVertexColorData> colorData;
  std::vector<GpuVertexTexCoordData> texCoordData;
  std::vector<GpuVertexNormalData> normalData;
  std::vector<GpuAnimationData> animationData;
};

}
