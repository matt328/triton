#version 460

layout(binding = 0) uniform UniformBufferObject {
   mat4 model;
   mat4 view;
   mat4 proj;
   uint textureId;
}
ubo;

struct ObjectData {
   mat4 model;
   uint textureId;
};

layout(std140, set = 2, binding = 0) readonly buffer ObjectBuffer {
   ObjectData objects[];
}
objectBuffer;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out uint textureId;

void main() {
   gl_Position =
       ubo.proj * ubo.view * objectBuffer.objects[gl_BaseInstance].model * vec4(inPosition, 1.0);
   fragColor = inColor;
   fragTexCoord = inTexCoord;
   textureId = objectBuffer.objects[gl_BaseInstance].textureId;
}
