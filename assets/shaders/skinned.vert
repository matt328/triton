#version 460

struct ObjectData {
   mat4 model;
   uint textureId;
   uint jointMId;
};

layout(std140, set = 1, binding = 0) readonly buffer ObjectBuffer {
   ObjectData objects[];
}
objectBuffer;

layout(set = 2, binding = 0) uniform CamerData {
   mat4 view;
   mat4 proj;
   mat4 viewProj;
}
camData;

layout(std430, set = 3, binding = 0) readonly buffer JointMatrices {
   mat4 jointMatrices[];
}
jointMatrices;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in uvec4 inJoints;
layout(location = 5) in vec4 inWeights;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out uint textureId;

void main() {

   ObjectData thisObject = objectBuffer.objects[gl_BaseInstance];

   mat4 skinMat = inWeights.x * jointMatrices.jointMatrices[thisObject.jointMId + int(inJoints.x)] +
                  inWeights.y * jointMatrices.jointMatrices[thisObject.jointMId + int(inJoints.y)] +
                  inWeights.z * jointMatrices.jointMatrices[thisObject.jointMId + int(inJoints.z)] +
                  inWeights.w * jointMatrices.jointMatrices[thisObject.jointMId + int(inJoints.w)];

   gl_Position = camData.proj * camData.view * thisObject.model * vec4(inPosition, 1.0);
   fragColor = inColor;
   fragTexCoord = inTexCoord;
   textureId = thisObject.textureId;
}

/*
   TODO: Figure out why this test file is jacking up the animated vertices
   probably make a ui to control the animation, at least play/pause it and maybe a slider to
   move through the timeline
*/