#pragma once

#include "cm/RenderData.hpp"
#include "geo/GeometryData.hpp"
#include "cm/Handles.hpp"
#include "vk/VkResourceManager.hpp"
#include <cstddef>

namespace tr {

struct BufferEntry {
  size_t vertexOffset;
  size_t vertexCount;
  size_t indexOffset;
  size_t indexCount;
};

/// InstanceData is passed into the compute shader along with camera data (frustum) and ObjectData
/// (AABB, model matrix, whatever is needed for culling) to generate the draw commands.
struct InstanceData2 {
  uint32_t indexCount;
  uint32_t instanceCount;
  uint32_t firstIndex;
  uint32_t instanceID;
  size_t objectDataId;
};

class MeshBufferManager {
public:
  explicit MeshBufferManager(std::shared_ptr<VkResourceManager> newResourceManager);
  ~MeshBufferManager() = default;

  MeshBufferManager(const MeshBufferManager&) = delete;
  MeshBufferManager(MeshBufferManager&&) = delete;
  auto operator=(const MeshBufferManager&) -> MeshBufferManager& = delete;
  auto operator=(MeshBufferManager&&) -> MeshBufferManager& = delete;

  auto addMesh(const GeometryData& geometryData) -> MeshHandle;
  auto removeMesh(MeshHandle meshHandle) -> void;

  [[nodiscard]] auto getInstanceData(std::vector<GpuMeshData> meshHandles)
      -> std::vector<InstanceData2>;

private:
  std::shared_ptr<VkResourceManager> resourceManager;

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
