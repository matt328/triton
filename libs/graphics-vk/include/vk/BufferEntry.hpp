#pragma once

namespace tr {

struct BufferEntry {
  uint32_t indexCount;
  uint32_t indexOffset;
  uint32_t indexSize;
  uint32_t vertexCount;
  uint32_t vertexOffset;
  uint32_t vertexSize;
};

/// InstanceData is passed into the compute shader along with camera data (frustum) and
/// GpuObjectData (AABB, model matrix, whatever is needed for culling) to generate the draw
/// commands. These parameters index into the large vertex and index buffers.
struct GpuBufferEntry {
  uint32_t indexCount;
  uint32_t firstIndex;
  uint32_t vertexOffset;
  uint32_t instanceCount;
  uint32_t firstInstance;
  uint32_t padding;
};

struct GeometryRegion {
  uint32_t indexCount;
  uint32_t firstIndex;
  uint32_t vertexOffset;
  uint32_t instanceCount;
  uint32_t firstInstance;
  uint32_t padding;
};
}
