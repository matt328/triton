#pragma once

#include "cm/ObjectData.hpp"

namespace tr {

struct GpuMeshData {
  MeshHandle handle{};
  Topology topology{Topology::Triangles};

  friend void swap(GpuMeshData& first, GpuMeshData& second) noexcept {
    using std::swap;
    swap(first.handle, second.handle);
    swap(first.topology, second.topology);
  }
};

struct alignas(ALIGNMENT) GpuInstanceData {
  glm::mat4 modelMatrix;
  alignas(4) uint32_t visible;
};

struct RenderData {
  PushConstants pushConstants{};
  CameraData cameraData{};
  std::vector<ObjectData> objectData;
  std::vector<GpuMeshData> staticMeshData;
  std::vector<GpuMeshData> terrainMeshData;
  std::vector<GpuMeshData> skinnedMeshData;
  std::vector<GpuAnimationData> animationData;

  std::vector<GpuMeshData> staticGpuMeshData;
};

}
