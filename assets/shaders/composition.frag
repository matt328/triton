#version 460
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec2 fragTexCoord;
// layout(location = 1) flat in uint textureId;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D texSampler[];

void main() {
  vec4 sceneColor = texture(texSampler[0], fragTexCoord);
  vec4 uiColor = texture(texSampler[1], fragTexCoord);

  outColor = mix(sceneColor, uiColor, uiColor.a);
}
