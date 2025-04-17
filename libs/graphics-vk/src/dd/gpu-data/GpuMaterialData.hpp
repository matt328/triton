#pragma once

namespace tr {

// Global Buffers
struct GpuObjectData {
  glm::mat4 modelMatrix;
  uint32_t geometryRegionId;
  uint32_t materialId;
  uint32_t animationId;
};

struct GpuAnimationData {
  glm::mat4 jointMatrices;
};

struct GpuMaterialData {
  glm::vec4 baseColor;
  uint32_t albedoTextureId;
};

using GpuObjectIndexData = uint32_t;
using GpuObjectCountData = uint32_t;

struct GpuGeometryRegionData {
  uint32_t indexCount;
  uint32_t firstIndex;
  uint32_t vertexOffset;
  uint32_t instanceCount;
  uint32_t firstInstance;
  uint32_t padding;
};

struct GpuIndirectCommand : public vk::DrawIndexedIndirectCommand {};

}
