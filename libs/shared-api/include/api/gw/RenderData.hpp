#pragma once

#include "api/gfx/GpuMaterialData.hpp"
#include "api/gfx/GpuTerrainDefinition.hpp"
#include "api/gfx/GpuObjectData.hpp"
#include "api/gfx/GpuCameraData.hpp"
#include "api/gw/RenderMeshData.hpp"

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
