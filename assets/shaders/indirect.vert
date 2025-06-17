#version 460
#extension GL_EXT_buffer_reference : enable
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#define INVALID_OFFSET 0xFFFFFFFFu

layout(push_constant) uniform PushConstants {
  uint64_t resourceTableAddress;
  uint64_t frameDataAddress;
}
pushConstants;

layout(buffer_reference, scalar) buffer ResourceTable {
  uint64_t objectDataBufferAddress;
  uint64_t objectPositionsAddress;
  uint64_t objectRotationsAddress;
  uint64_t objectScalesAddress;
  uint64_t regionBufferAddress;
  uint64_t indexBufferAddress;
  uint64_t positionBufferAddress;
  uint64_t colorBufferAddress;
  uint64_t texCoordBufferAddress;
  uint64_t normalBufferAddress;
  uint64_t animationBufferAddress;
  uint64_t materialBufferAddress;
  uint64_t indirectCommandAddress;
  uint64_t indirectCountAddress;
};

layout(buffer_reference, scalar) buffer FrameDataBuffer {
  mat4 view;
  mat4 projection;
  vec4 cameraPosition;
  float time;
  uint maxObjects;
  float _pad0;
  float _pad1;
};

layout(buffer_reference, std430) buffer IndexBuffer {
  uint index[];
};

layout(buffer_reference, std430) buffer PositionBuffer {
  vec3 positions[];
};

layout(buffer_reference, std430) buffer ColorBuffer {
  vec4 colors[];
};

layout(buffer_reference, std430) buffer TexCoordBuffer {
  vec2 texCoords[];
};

layout(buffer_reference, std430) buffer NormalBuffer {
  vec3 normals[];
};

struct GpuGeometryRegionData {
  uint indexCount;
  uint indexOffset;
  uint positionOffset;
  uint colorOffset;
  uint texCoordOffset;
  uint normalOffset;
};

layout(buffer_reference, std430) buffer RegionBuffer {
  GpuGeometryRegionData regions[];
};

struct GpuObjectData {
  uint transformIndex;
  uint rotationIndex;
  uint scaleIndex;
  uint geometryRegionId;
  uint materialId;
  uint animationId;
};

layout(buffer_reference, std430) buffer ObjectDataBuffer {
  GpuObjectData objects[];
};

layout(buffer_reference, scalar) buffer GpuPositionDataBuffer {
  vec3 position;
};

layout(buffer_reference, scalar) buffer GpuRotationDataBuffer {
  vec4 rotation;
};

layout(buffer_reference, scalar) buffer GpuScaleDataBuffer {
  vec3 scale;
};

layout(location = 0) out vec2 v_texCoord;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec4 v_color;

vec3 applyQuaternion(vec4 q, vec3 v) {
  return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

void main() {

  ResourceTable resourceTable = ResourceTable(pushConstants.resourceTableAddress);
  FrameDataBuffer frameData = FrameDataBuffer(pushConstants.frameDataAddress);

  ObjectDataBuffer objectDataBuf = ObjectDataBuffer(resourceTable.objectDataBufferAddress);
  RegionBuffer regionBuf = RegionBuffer(resourceTable.regionBufferAddress);

  GpuObjectData object = objectDataBuf.objects[gl_InstanceIndex + gl_BaseInstance];

  GpuPositionDataBuffer positionDataBuffer =
      GpuPositionDataBuffer(resourceTable.objectPositionsAddress);
  vec3 objectPosition = positionDataBuffer[object.transformIndex].position;

  GpuRotationDataBuffer rotationDataBuffer =
      GpuRotationDataBuffer(resourceTable.objectRotationsAddress);
  vec4 objectRotation = rotationDataBuffer[object.rotationIndex].rotation;

  GpuScaleDataBuffer scaleDataBuffer = GpuScaleDataBuffer(resourceTable.objectScalesAddress);
  vec3 objectScale = scaleDataBuffer[object.scaleIndex].scale;

  GpuGeometryRegionData region = regionBuf.regions[object.geometryRegionId];

  IndexBuffer indexBuf = IndexBuffer(resourceTable.indexBufferAddress);
  uint vertexIndex = indexBuf.index[region.indexOffset + gl_VertexIndex];

  PositionBuffer posBuf = PositionBuffer(resourceTable.positionBufferAddress);
  vec3 position = posBuf.positions[region.positionOffset + vertexIndex];

  TexCoordBuffer texBuf = TexCoordBuffer(resourceTable.texCoordBufferAddress);
  vec2 texCoord = texBuf.texCoords[region.texCoordOffset + vertexIndex];

  vec3 normal = vec3(1.0, 1.0, 1.0);
  if (region.normalOffset != INVALID_OFFSET) {
    NormalBuffer normBuf = NormalBuffer(resourceTable.normalBufferAddress);
    normal = normBuf.normals[region.normalOffset + vertexIndex];
  }

  vec4 color = vec4(1.0, 1.0, 1.0, 1.0);
  if (region.colorOffset != INVALID_OFFSET) {
    ColorBuffer colorBuf = ColorBuffer(resourceTable.colorBufferAddress);
    vec4 color = colorBuf.colors[region.colorOffset + vertexIndex];
  }

  vec3 scaled = objectScale * position;
  vec3 rotated = applyQuaternion(objectRotation, scaled);
  vec3 transformed = rotated + objectPosition;

  gl_Position = vec4(transformed, 1.0);
  v_texCoord = texCoord;
  v_normal = normal;
  v_color = color;
}
