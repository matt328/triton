#pragma once

namespace tr {

constexpr uint32_t INVALID_OFFSET = std::numeric_limits<uint32_t>::max(); // 0xFFFFFFFF

// ObjectData Buffer
struct GpuObjectData {
  glm::mat4 modelMatrix;
  uint32_t geometryRegionId;
  uint32_t materialId;
  uint32_t animationId;
};

/// Describes a single Mesh in the GpuVertexData struct by indexing into the GpuIndexData buffer
struct GpuGeometryRegionData {
  uint32_t indexCount = 0;
  uint32_t indexOffset = INVALID_OFFSET;

  uint32_t positionOffset = 0;
  uint32_t colorOffset = INVALID_OFFSET;
  uint32_t texCoordOffset = INVALID_OFFSET;
  uint32_t normalOffset = INVALID_OFFSET;
};

// Typical Index Data each index 'indexes' into the GpuVertexData buffer
struct GpuIndexData {
  uint32_t index;
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
  glm::vec3 normal;
};

struct GpuAnimationData {
  glm::mat4 jointMatrices;
};

struct GpuMaterialData {
  glm::vec4 baseColor;
  uint32_t albedoTextureId;
};

using GpuObjectCountData = uint32_t;

struct GpuIndirectCommand : public vk::DrawIndexedIndirectCommand {};

/*
  GpuIndirectCommand fields:
  indexCount * instanceCount == how many times to run the vertex shader
  instanceCount is tracked in the loop by gl_InstanceIndex
  indexCount is tracked in the loop by gl_VertexIndex
  firstInstance sets the base value of gl_InstanceIndex
  firstIndex and vertexOffset are ignored by vulkan when no vertex/index buffer is bound

  gl_VertexIndex ranges from 0 to indexCount - 1
  gl_InstanceIndex ranges from firstInstance to firstInstance + instanceCount - 1
*/

}
