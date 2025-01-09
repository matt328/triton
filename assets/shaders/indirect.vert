#version 460
#extension GL_EXT_buffer_reference : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_EXT_shader_explicit_arithmetic_types : require

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec4 inColor;
layout(location = 4) in uvec4 inJoints;
layout(location = 5) in vec4 inWeights;
layout(location = 6) in vec4 inTangents;

layout(location = 0) out vec4 fragColor;

layout(push_constant) uniform PushConstants {
  uint drawID;
  uint64_t objectDataAddress;
  uint64_t cameraDataAddress;
  uint objectDataLength;
}
pc;

struct ObjectData {
  mat4 modelMatrix;
  uint64_t textureId;
  uint animationDataIndex;
  uint _padding;
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

void main() {

  ObjectDataBuffer objectDataBuffer = ObjectDataBuffer(pc.objectDataAddress);

  if (gl_InstanceIndex >= pc.objectDataLength) {
    return;
  }

  CameraDataBuffer camData = CameraDataBuffer(pc.cameraDataAddress);

  mat4 model = objectDataBuffer.objectData[gl_DrawID].modelMatrix;
  vec4 worldPos = camData.proj * camData.view * model * vec4(inPosition, 1.0);

  gl_Position = worldPos;
  fragColor = inColor;
}
