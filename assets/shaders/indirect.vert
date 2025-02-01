#version 460
#extension GL_EXT_buffer_reference : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable
#extension GL_EXT_shader_explicit_arithmetic_types : require

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in uvec4 inJoints;
layout(location = 3) in vec4 inWeights;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out uint textureId;

layout(push_constant) uniform PushConstants {
  uint64_t objectDataAddress;
  uint64_t cameraDataAddress;
  uint64_t objectDataIndexAddress;
  uint64_t animationDataAddress;
  uint drawID;
  uint objectCount;
}
pc;

struct ObjectData {
  mat4 modelMatrix;
  uint textureId;
  uint animationDataIndex;
  uint _padding;
};

layout(buffer_reference, std430) readonly buffer ObjectDataBuffer {
  ObjectData objectData[];
};

layout(buffer_reference, std430) readonly buffer AnimationDataBuffer {
  mat4 jointMatrices[];
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
  AnimationDataBuffer animationData = AnimationDataBuffer(pc.animationDataAddress);

  uint objectIndex = objectDataIndexBuffer.index[gl_DrawID];

  ObjectData currentObject = objectDataBuffer.objectData[objectIndex];

  mat4 skinMat =
      inWeights.x *
          animationData.jointMatrices[currentObject.animationDataIndex + int(inJoints.x)] +
      inWeights.y *
          animationData.jointMatrices[currentObject.animationDataIndex + int(inJoints.y)] +
      inWeights.z *
          animationData.jointMatrices[currentObject.animationDataIndex + int(inJoints.z)] +
      inWeights.w * animationData.jointMatrices[currentObject.animationDataIndex + int(inJoints.w)];

  mat4 model = currentObject.modelMatrix;
  vec4 worldPos = camData.proj * camData.view * model * skinMat * vec4(inPosition, 1.0);

  gl_Position = worldPos;
  fragTexCoord = inTexCoord;
  textureId = currentObject.textureId;
}
