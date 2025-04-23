#version 450

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragUV;

layout(set = 0, binding = 0) readonly buffer ObjectDataBuffer {
  GpuObjectData objects[];
};

layout(set = 0, binding = 1) readonly buffer GeometryCommandBuffer {
  GpuGeometryCommandData geometryCommands[];
};

layout(set = 0, binding = 2) readonly buffer PositionBuffer {
  vec3 positions[];
};

layout(set = 0, binding = 3) readonly buffer NormalBuffer {
  vec3 normals[];
};

layout(set = 0, binding = 4) readonly buffer UVBuffer {
  vec2 uvs[];
};

layout(set = 0, binding = 5) readonly buffer IndexBuffer {
  uint indices[];
};

layout(push_constant) uniform PushConstants {
  mat4 viewProj;
}
pc;

void main() {
  uint instanceIndex = gl_InstanceIndex;
  GpuObjectData object = objects[instanceIndex];
  GpuGeometryCommandData geomCmd = geometryCommands[object.geometryCommandId];

  uint drawIndex = geomCmd.firstIndex + gl_VertexIndex;
  uint vertexIndex = indices[drawIndex] + geomCmd.vertexOffset;

  vec3 pos = positions[vertexIndex];
  vec3 norm = normals[vertexIndex];
  vec2 uv = uvs[vertexIndex];

  gl_Position = pc.viewProj * object.modelMatrix * vec4(pos, 1.0);
  fragNormal = mat3(object.modelMatrix) * norm;
  fragUV = uv;
}
