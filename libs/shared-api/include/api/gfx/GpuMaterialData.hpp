#pragma once

namespace tr {

constexpr uint32_t INVALID_OFFSET = std::numeric_limits<uint32_t>::max(); // 0xFFFFFFFF

/// ObjectData Buffer
struct GpuObjectData {
  uint32_t transformIndex;
  uint32_t rotationIndex;
  uint32_t scaleIndex;

  uint32_t geometryRegionId;
  uint32_t materialId;
  uint32_t animationId;

  uint32_t _padding = 0;
};

struct GpuTransformData {
  glm::vec3 position;
  float _pad0 = 0.f;
};

struct GpuRotationData {
  glm::quat rotation;
};

struct GpuScaleData {
  glm::vec3 scale;
  float _pad0 = 0.f;
};

/// Describes a single Mesh in the GpuVertex*Data struct by indexing into the GpuIndexData buffer
struct GpuGeometryRegionData {
  uint32_t indexCount = 0;
  uint32_t indexOffset = INVALID_OFFSET;

  uint32_t positionOffset = 0;
  uint32_t colorOffset = INVALID_OFFSET;
  uint32_t texCoordOffset = INVALID_OFFSET;
  uint32_t normalOffset = INVALID_OFFSET;
};

// Typical Index Data each index 'indexes' into the GpuVertex*Data buffer
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

struct GpuIndirectCommand {
  uint32_t indexCount;
  uint32_t instanceCount;
  uint32_t firstIndex;
  int32_t vertexOffset;
  uint32_t firstInstance;
};

struct GeometryData {
  std::vector<GpuIndexData> indexData{};
  std::vector<GpuVertexPositionData> positionData{};
  std::vector<GpuVertexColorData> colorData{};
  std::vector<GpuVertexTexCoordData> texCoordData;
  std::vector<GpuVertexNormalData> normalData{};
  std::vector<GpuAnimationData> animationData{};
};

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
