#version 450
#extension GL_EXT_buffer_reference : enable
#extension GL_EXT_buffer_reference_uvec2 : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable

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
   uint64_t baseAddress;
   uint64_t cameraDataAddress;
}
pc;

layout(buffer_reference, std430) readonly buffer InstanceBuffer {
   mat4 modelMatrix[];
};

layout(buffer_reference, std430) readonly buffer CameraDataBuffer {
   mat4 view;
   mat4 proj;
   mat4 viewProj;
};

void main() {
   InstanceBuffer instances = InstanceBuffer(pc.baseAddress);
   CameraDataBuffer camData = CameraDataBuffer(pc.cameraDataAddress);

   mat4 model = instances.modelMatrix[gl_InstanceIndex];
   vec4 worldPos = camData.proj * camData.view * model * vec4(inPosition, 1.0);

   gl_Position = worldPos;
   fragColor = inColor;
}
