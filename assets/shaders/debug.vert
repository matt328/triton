#version 460

struct ObjectData {
  mat4 model;
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

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
  gl_Position = (camData.proj * camData.view * objectBuffer.objects[gl_BaseInstance].model) *
                vec4(inPosition, 1.0);
  fragColor = inColor;
}
