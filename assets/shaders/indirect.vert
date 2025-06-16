#version 460
#extension GL_EXT_buffer_reference : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

layout(push_constant) uniform PushConstants {
  uint64_t objectDataBufferAddress;
  uint64_t regionDataAddress;
  uint64_t indexBufferAddress;
  uint64_t positionBufferAddress;
  uint64_t colorBufferAddress;
  uint64_t texCoordBufferAddress;
  uint64_t normalBufferAddress;
  uint64_t animationBufferAddress;
  uint64_t materialBufferAddress;
  uint geometryRegionIndex;
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

layout(location = 0) out vec2 v_texCoord;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec4 v_color;

void main() {
  ObjectDataBuffer objectDataBuf = ObjectDataBuffer(objectDataBufferAddress);

  RegionBuffer regionBuf = RegionBuffer(regionDataAddress);

  GpuObjectData object = objectDataBuf.objects[gl_InstanceIndex + gl_BaseInstance];

  GpuGeometryRegionData region = regionBuf.regions[object.geometryRegionId];

  IndexBuffer indexBuf = IndexBuffer(indexBufferAddress);
  uint vertexIndex = indexBuf.index[region.indexOffset + gl_VertexIndex];

  PositionBuffer posBuf = PositionBuffer(positionBufferAddress);
  vec3 position = posBuf.positions[region.positionOffset + vertexIndex];

  TexCoordBuffer texBuf = TexCoordBuffer(texCoordBufferAddress);
  vec2 texCoord = texBuf.texCoords[region.texCoordOffset + vertexIndex];

  NormalBuffer normBuf = NormalBuffer(normalBufferAddress);
  vec3 normal = normBuf.normals[region.normalOffset + vertexIndex];

  ColorBuffer colorBuf = ColorBuffer(colorBufferAddress);
  vec4 color = colorBuf.colors[region.colorOffset + vertexIndex];

  // TODO: pull in object TRS buffers and look them up to multiply the position

  gl_Position = vec4(position, 1.0);
  v_texCoord = texCoord;
  v_normal = normal;
  v_color = color;
}
