#pragma once

#include "api/gfx/GeometryEntry.hpp"
namespace tr {

// Global Buffers
struct GpuObjectData {
  glm::mat4 modelMatrix;
  uint32_t geometryRegionId;
  uint32_t materialId;
  uint32_t animationId;
};

struct GpuVertexPositionData {
  glm::vec3 position;
};

struct GpuVertexColorData {
  glm::vec4 color;
};

struct GpuVertexTexCoordData {
  glm::vec2 texCoords;
};

struct GpuVertexNormalData {
  glm::vec4 normal;
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

/*
  Is this actually needed?
  What information needs to be set in the IndirectCommand buffer, and what variables do those become
  in the shader programs?
*/
/// Information about where the vertices and indices are in the global geometry buffer that gets
/// turned into a DrawIndirectCommand by the compute shader.
struct GpuGeometryCommandData {
  uint32_t indexCount;
  uint32_t firstIndex;
  uint32_t vertexOffset;
  uint32_t instanceCount;
  uint32_t firstInstance;
};

struct GpuIndirectCommand : public vk::DrawIndexedIndirectCommand {};

}
