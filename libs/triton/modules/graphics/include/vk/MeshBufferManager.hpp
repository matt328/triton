#pragma once

#include "cm/Rando.hpp"
#include "cm/RenderMeshData.hpp"
#include "geo/GeometryData.hpp"
#include "cm/Handles.hpp"
#include "vk/ResourceManagerHandles.hpp"
#include "vk/BufferEntry.hpp"

namespace tr {

class IBufferManager;
class Buffer;

struct Block {
  uint32_t offset;
  uint32_t size;
};

struct FilledBlock : public Block {
  uint32_t itemCount;
};

class MeshBufferManager {
public:
  explicit MeshBufferManager(std::shared_ptr<IBufferManager> newBufferManager,
                             size_t vertexSize,
                             std::string_view bufferName);
  ~MeshBufferManager();

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
  std::shared_ptr<IBufferManager> bufferManager;

  const size_t vbStride;
  const size_t ibStride;

  size_t vbMaxSize;
  size_t ibMaxSize;

  size_t vbMaxAllocatedOffset = 0;
  size_t ibMaxAllocatedOffset = 0;

  const float vbMaxLoad;
  const float ibMaxLoad;

  BufferHandle vbHandle;
  BufferHandle ibHandle;

  MapKey bufferKeygen{};
  std::unordered_map<size_t, BufferEntry> bufferEntries;

  std::vector<GpuBufferEntry> gpuBufferEntryList;

  std::vector<Block> emptyIndexBlocks;
  std::vector<Block> emptyVertexBlocks;

  std::vector<FilledBlock> filledIndexBlocks;
  std::vector<FilledBlock> filledVertexBlocks;

  auto testPrivateMethod() -> void;

  auto findEmptyBlock(const IGeometryData& geometryData, const std::vector<Block>& blocks)
      -> std::optional<Block>;
};

}
