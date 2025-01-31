#pragma once

#include "cm/RenderMeshData.hpp"
#include "geo/GeometryData.hpp"
#include "cm/Handles.hpp"
#include "ResourceManagerHandles.hpp"
#include "BufferEntry.hpp"

namespace tr {

class BufferManager;
class Buffer;

class MeshBufferManager {
public:
  explicit MeshBufferManager(std::shared_ptr<BufferManager> newBufferManager,
                             size_t vertexSize,
                             std::string_view bufferName);
  ~MeshBufferManager() = default;

  MeshBufferManager(const MeshBufferManager&) = delete;
  MeshBufferManager(MeshBufferManager&&) = delete;
  auto operator=(const MeshBufferManager&) -> MeshBufferManager& = delete;
  auto operator=(MeshBufferManager&&) -> MeshBufferManager& = delete;

  auto addMesh(const IGeometryData& geometryData) -> MeshHandle;
  auto removeMesh(MeshHandle meshHandle) -> void;

  [[nodiscard]] auto getGpuBufferEntries(const std::vector<RenderMeshData>& meshDataList)
      -> std::vector<GpuBufferEntry>&;

  [[nodiscard]] auto getVertexBufferHandle() const -> BufferHandle;
  [[nodiscard]] auto getIndexBufferHandle() const -> BufferHandle;

  [[nodiscard]] auto getBuffers() const -> std::tuple<Buffer&, Buffer&>;

private:
  std::shared_ptr<BufferManager> bufferManager;

  size_t vertexBufferMaxSize;
  size_t indexBufferMaxSize;

  size_t vertexBufferCurrentSize = 0;
  size_t indexBufferCurrentSize = 0;

  float vertexBufferMaxLoad;
  float indexBufferMaxLoad;

  BufferHandle vertexBufferHandle;
  BufferHandle indexBufferHandle;

  std::vector<BufferEntry> bufferEntries;

  std::vector<GpuBufferEntry> gpuBufferEntryList;

  auto testPrivateMethod() -> void;
};

}
