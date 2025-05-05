#version 460
#extension GL_EXT_buffer_reference : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_EXT_shader_explicit_arithmetic_types : require

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out uint textureId;

layout(push_constant) uniform PushConstants {
  uint drawID;
  uint64_t objectDataAddress;
  uint64_t cameraDataAddress;
  uint64_t objectDataIndexAddress;
  uint objectCount;
}
pc;

struct ObjectData {
  mat4 modelMatrix;
  uint textureId;
  uint _padding[3];
};

layout(buffer_reference, std430) readonly buffer ObjectDataBuffer {
  ObjectData objectData[];
};

layout(buffer_reference, std430) readonly buffer CameraDataBuffer {
  mat4 view;
  mat4 proj;
  mat4 viewProj;
  vec4 position;
};

layout(buffer_reference, std430) readonly buffer ObjectDataIndexBuffer {
  uint index[];
};

void main() {

  ObjectDataBuffer objectDataBuffer = ObjectDataBuffer(pc.objectDataAddress);
  CameraDataBuffer camData = CameraDataBuffer(pc.cameraDataAddress);
  ObjectDataIndexBuffer objectDataIndexBuffer = ObjectDataIndexBuffer(pc.objectDataIndexAddress);

  uint objectIndex = objectDataIndexBuffer.index[gl_DrawID];

  mat4 model = objectDataBuffer.objectData[objectIndex].modelMatrix;
  vec4 worldPos = camData.proj * camData.view * model * vec4(0.0, 0.0, 0.0, 1.0);

  gl_Position = worldPos;
  fragTexCoord = vec2(0.0, 0.0);
  textureId = objectDataBuffer.objectData[objectIndex].textureId;
}
