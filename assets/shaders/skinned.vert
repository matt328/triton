#version 460

struct ObjectData {
   mat4 model;
   uint textureId;
};

layout(std140, set = 1, binding = 0) readonly buffer ObjectBuffer {
   ObjectData objects[];
}
objectBuffer;

layout(set = 2, binding = 0) uniform CameraData {
   mat4 view;
   mat4 proj;
   mat4 viewProj;
}
camData;

layout(std430, set = 3, binding = 0) readonly buffer JointMatrices {
   mat4 jointMatrices[];
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec4 inJoints;
layout(location = 5) in vec4 inWeights;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out uint textureId;

void main() {
   gl_Position = (camData.proj * camData.view * objectBuffer.objects[gl_BaseInstance].model) *
                 vec4(inPosition, 1.0);
   fragColor = inColor;
   fragTexCoord = inTexCoord;
   textureId = objectBuffer.objects[gl_BaseInstance].textureId;
}
