#version 460
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) flat in uint textureId;

layout(location = 0) out vec4 outColor;

void main() {
  outColor = vec4(1.f, 1.f, 1.f, 1.f);
}
