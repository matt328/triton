#version 460
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_explicit_arithmetic_types : require

layout(location = 0) out vec4 vColor;
layout(location = 1) out vec2 vUV;
layout(location = 2) out vec3 vNormal;

// --- Buffer definitions (referencable structs) ---

struct GpuObjectData {
  uint transformIndex;
  uint rotationIndex;
  uint scaleIndex;

  uint geometryRegionId;
  uint materialId;
  uint animationId;
};

struct GpuGeometryRegionData {
  uint indexCount;
  uint indexOffset;

  uint positionOffset;
  uint colorOffset;
  uint texCoordOffset;
  uint normalOffset;
};

// Buffer references
layout(buffer_reference, scalar) buffer ObjectDataBuffer {
  GpuObjectData objects[];
};

layout(buffer_reference, scalar) buffer TransformBuffer {
  vec4 transforms[]; // packed: vec4(position.xyz, _pad)
};

layout(buffer_reference, scalar) buffer RotationBuffer {
  vec4 rotations[]; // quaternion
};

layout(buffer_reference, scalar) buffer ScaleBuffer {
  vec4 scales[]; // packed: vec4(scale.xyz, _pad)
};

layout(buffer_reference, scalar) buffer GeometryRegionBuffer {
  GpuGeometryRegionData regions[];
};

layout(buffer_reference, scalar) buffer IndexBuffer {
  uint indices[];
};

layout(buffer_reference, scalar) buffer PositionBuffer {
  vec3 positions[];
};

layout(buffer_reference, scalar) buffer ColorBuffer {
  vec4 colors[];
};

layout(buffer_reference, scalar) buffer TexCoordBuffer {
  vec2 texCoords[];
};

layout(buffer_reference, scalar) buffer NormalBuffer {
  vec3 normals[];
};

// --- Push constants ---

layout(push_constant) uniform PushConstants {
  mat4 viewProj;

  ObjectDataBuffer objBuf;
  TransformBuffer transformBuf;
  RotationBuffer rotationBuf;
  ScaleBuffer scaleBuf;
  GeometryRegionBuffer regionBuf;
  IndexBuffer indexBuf;
  PositionBuffer posBuf;
  ColorBuffer colorBuf;
  TexCoordBuffer uvBuf;
  NormalBuffer normalBuf;
}
pc;

// --- Helper function to build TRS matrix ---

mat4 trs(vec3 t, vec4 q, vec3 s) {
  float x = q.x, y = q.y, z = q.z, w = q.w;
  mat3 R = mat3(1 - 2 * y * y - 2 * z * z,
                2 * x * y - 2 * w * z,
                2 * x * z + 2 * w * y,
                2 * x * y + 2 * w * z,
                1 - 2 * x * x - 2 * z * z,
                2 * y * z - 2 * w * x,
                2 * x * z - 2 * w * y,
                2 * y * z + 2 * w * x,
                1 - 2 * x * x - 2 * y * y);
  return mat4(vec4(R[0] * s.x, 0.0), vec4(R[1] * s.y, 0.0), vec4(R[2] * s.z, 0.0), vec4(t, 1.0));
}

// --- Main ---

// Main gets called IndirectCommand.indexCount number of times, gl_InstanceIndex tracks this number

void main() {
  GpuObjectData obj = pc.objBuf.objects[gl_InstanceIndex];
  GpuGeometryRegionData region = pc.regionBuf.regions[obj.geometryRegionId];

  uint idx = pc.indexBuf.indices[region.indexOffset + gl_VertexIndex];

  vec3 pos = pc.posBuf.positions[region.positionOffset + idx];
  vec4 col =
      region.colorOffset != 0xFFFFFFFFu ? pc.colorBuf.colors[region.colorOffset + idx] : vec4(1.0);
  vec2 uv = region.texCoordOffset != 0xFFFFFFFFu ? pc.uvBuf.texCoords[region.texCoordOffset + idx]
                                                 : vec2(0.0);
  vec3 nor = region.normalOffset != 0xFFFFFFFFu ? pc.normalBuf.normals[region.normalOffset + idx]
                                                : vec3(0.0, 1.0, 0.0);

  vec3 t = pc.transformBuf.transforms[obj.transformIndex].xyz;
  vec4 q = pc.rotationBuf.rotations[obj.rotationIndex];
  vec3 s = pc.scaleBuf.scales[obj.scaleIndex].xyz;
  mat4 model = trs(t, q, s);

  gl_Position = pc.viewProj * model * vec4(pos, 1.0);
  vColor = col;
  vUV = uv;
  vNormal = mat3(model) * nor;
}
