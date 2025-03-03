#pragma once

#include "GpuTerrainDefinition.hpp"
#include "cm/GpuAnimationData.hpp"
#include "cm/GpuObjectData.hpp"
#include "cm/GpuCameraData.hpp"
#include "cm/RenderMeshData.hpp"

namespace tr {

struct RenderData {
  GpuCameraData cameraData{};
  std::vector<StaticGpuObjectData> objectData;
  std::vector<DynamicGpuObjectData> dynamicObjectData;
  std::vector<TerrainGpuObjectData> terrainObjectData;
  std::vector<RenderMeshData> staticMeshData;
  std::vector<RenderMeshData> terrainMeshData;
  std::vector<RenderMeshData> dynamicMeshData;
  std::vector<GpuAnimationData> animationData;
  std::vector<RenderMeshData> staticGpuMeshData;

  GpuTerrainDefinition terrainDefinition;
};

}
