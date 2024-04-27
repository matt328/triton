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

layout(push_constant) uniform PushConstants {
   vec4 lightPosition;
   vec4 params;
}
pushConstants;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out uint textureId;
layout(location = 3) out vec3 camPosition;
layout(location = 4) out vec3 normal;
layout(location = 5) out vec3 position;
layout(location = 6) out vec4 lightPosition;
layout(location = 7) out float specularPower;

void main() {
   mat4 model = objectBuffer.objects[gl_BaseInstance].model;
   gl_Position = (camData.proj * camData.view * model) * vec4(inPosition, 1.0);
   fragColor = inColor;
   fragTexCoord = inTexCoord;
   textureId = objectBuffer.objects[gl_BaseInstance].textureId;
   camPosition = vec3(0.0, 0.0, 0.0);
   normal = inNormal;
   position = inPosition;
   lightPosition = pushConstants.lightPosition;
   specularPower = pushConstants.params[0];
}
