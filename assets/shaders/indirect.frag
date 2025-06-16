#version 460
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec2 v_texCoord;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec4 v_color;

layout(location = 0) out vec4 outColor;

void main() {
  outColor = vec4(1.f, 1.f, 1.f, 1.f);
}
