#pragma once

namespace tr {

constexpr uint32_t INVALID_OFFSET = std::numeric_limits<uint32_t>::max(); // 0xFFFFFFFF

struct GpuFrameData {
  glm::mat4 view;
  glm::mat4 projection;
  glm::vec4 cameraPosition;
  float time;
  uint32_t maxObjects;
};

struct GpuResourceTable {
  uint64_t objectDataBufferAddress{};
  uint64_t objectPositionsAddress{};
  uint64_t objectRotationsAddress{};
  uint64_t objectScalesAddress{};
  uint64_t regionBufferAddress{};
  uint64_t indexBufferAddress{};
  uint64_t positionBufferAddress{};
  uint64_t colorBufferAddress{};
  uint64_t texCoordBufferAddress{};
  uint64_t normalBufferAddress{};
  uint64_t animationBufferAddress{};
  uint64_t materialBufferAddress{};
  uint64_t indirectCommandAddress{};
  uint64_t indirectCountAddress{};
};

/// ObjectData Buffer
struct GpuObjectData {
  uint32_t transformIndex;
  uint32_t rotationIndex;
  uint32_t scaleIndex;

  uint32_t geometryRegionId;
  uint32_t materialId;
  uint32_t animationId;
};

struct GpuTransformData {
  glm::vec3 position;
};

struct GpuRotationData {
  glm::quat rotation;
};

struct GpuScaleData {
  glm::vec3 scale;
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

}
