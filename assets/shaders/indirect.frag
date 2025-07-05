#version 460
#extension GL_EXT_buffer_reference : enable
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

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

struct GpuObjectData {
  uint transformIndex;
  uint rotationIndex;
  uint scaleIndex;
  uint geometryRegionId;
  uint materialId;
  uint animationId;
};

struct GpuMaterialData {
  vec4 baseColor;
  uint albedoTextureId;
};

layout(buffer_reference, scalar) buffer ObjectDataBuffer {
  GpuObjectData objects[];
};

layout(buffer_reference, scalar) buffer MaterialDataBuffer {
  GpuMaterialData materials[];
};

layout(set = 0, binding = 0) uniform sampler2D texSampler[];

layout(location = 0) in vec2 v_texCoord;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec4 v_color;
layout(location = 3) flat in uint objectId;

layout(location = 0) out vec4 outColor;

void main() {

  ResourceTable resourceTable = ResourceTable(pushConstants.resourceTableAddress);
  MaterialDataBuffer materialDataBuffer = MaterialDataBuffer(resourceTable.materialBufferAddress);
  ObjectDataBuffer objectDataBuf = ObjectDataBuffer(resourceTable.objectDataBufferAddress);

  GpuObjectData object = objectDataBuf.objects[objectId];
  GpuMaterialData materialData = materialDataBuffer.materials[object.materialId];

  outColor = texture(texSampler[materialData.albedoTextureId], v_texCoord);
}
