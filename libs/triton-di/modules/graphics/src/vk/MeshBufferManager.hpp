#pragma once

#include "cm/RenderData.hpp"
#include "geo/GeometryData.hpp"
#include "cm/Handles.hpp"
#include "ResourceManagerHandles.hpp"

namespace tr {

struct BufferEntry {
  uint32_t indexCount;
  uint32_t firstIndex;
  uint32_t vertexOffset;
  uint32_t vertexCount;
};

/// InstanceData is passed into the compute shader along with camera data (frustum) and ObjectData
/// (AABB, model matrix, whatever is needed for culling) to generate the draw commands. These
/// parameters index into the large vertex and index buffers.
struct GpuBufferEntry {
  uint32_t indexCount;
  uint32_t firstIndex;
  uint32_t vertexOffset;
  uint32_t instanceCount;
  uint32_t firstInstance;
  uint32_t objectDataId;
};

class VkResourceManager;

class MeshBufferManager {
public:
  explicit MeshBufferManager(VkResourceManager* newResourceManager);
  ~MeshBufferManager() = default;

  MeshBufferManager(const MeshBufferManager&) = delete;
  MeshBufferManager(MeshBufferManager&&) = delete;
  auto operator=(const MeshBufferManager&) -> MeshBufferManager& = delete;
  auto operator=(MeshBufferManager&&) -> MeshBufferManager& = delete;

  auto addMesh(const GeometryData& geometryData) -> MeshHandle;
  auto removeMesh(MeshHandle meshHandle) -> void;

  [[nodiscard]] auto getGpuBufferEntries(const std::vector<GpuMeshData>& meshDataList)
      -> std::vector<GpuBufferEntry>;

  [[nodiscard]] auto getVertexBufferHandle() const -> BufferHandle;
  [[nodiscard]] auto getIndexBufferHandle() const -> BufferHandle;

private:
  VkResourceManager* resourceManager;

  size_t vertexBufferMaxSize;
  size_t indexBufferMaxSize;

  size_t vertexBufferCurrentSize = 0;
  size_t indexBufferCurrentSize = 0;

  float vertexBufferMaxLoad;
  float indexBufferMaxLoad;

  BufferHandle vertexBufferHandle;
  BufferHandle indexBufferHandle;

  std::vector<BufferEntry> bufferEntries;

  auto testPrivateMethod() -> void;
};

}
