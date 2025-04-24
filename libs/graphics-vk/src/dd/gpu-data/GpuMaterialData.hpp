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
  GpuGeometryCommandData:
    - instanceCount - number of instances to render
      - in shaders, use this as the stop condition when looping while gl_InstanceIndex <
        instanceCount
    - firstInstance - starting point for gl_InstanceIndex
    - firstIndex - startingPoint for gl_VertexIndex, used to pull vertex attributes
    - indexCount - number of indices used for the draw call, not used directly in shader code
    - vertexOffset - used specifically in shader code for offsetting into a shared vertex buffer
        index = indexBuffer[firstIndex + gl_VertexIndex];
        position = vertexBuffer[index + vertexOffset];

  - This struct gets put into a buffer. The compute culling will write its contents once per frame,
    and it will be used both internally by drawIndexedIndirectCount() and also referenced manually
    in shader code for vertex pulling.
  - I don't think we need a GpuGeometryCommandData, I think on the CPU side this is the
  GeometryEntry struct that gets passed into the Compute shader to produce the
  DrawIndexedIndirectCommand Buffer

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
