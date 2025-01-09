#pragma once

#include "cm/GpuAnimationData.hpp"
#include "cm/GpuObjectData.hpp"
#include "cm/GpuCameraData.hpp"
#include "cm/GpuMeshData.hpp"

namespace tr {

struct alignas(ALIGNMENT) GpuInstanceData {
  glm::mat4 modelMatrix;
  alignas(4) uint32_t visible;
};

struct RenderData {
  GpuCameraData cameraData{};
  std::vector<GpuObjectData> objectData;
  std::vector<GpuMeshData> staticMeshData;
  std::vector<GpuMeshData> terrainMeshData;
  std::vector<GpuMeshData> skinnedMeshData;
  std::vector<GpuAnimationData> animationData;

  std::vector<GpuMeshData> staticGpuMeshData;
};

}
