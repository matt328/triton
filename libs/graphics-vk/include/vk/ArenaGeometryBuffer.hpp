#pragma once

#include "api/gw/Handles.hpp"
#include "bk/Rando.hpp"
#include "api/gw/RenderMeshData.hpp"
#include "mem/ArenaBuffer.hpp"
#include "vk/BufferEntry.hpp"

namespace tr {

class IBufferManager;

class ArenaGeometryBuffer {
public:
  ArenaGeometryBuffer(IBufferManager* newBufferManager,
                      const ArenaGeometryBufferCreateInfo& createInfo);
  ~ArenaGeometryBuffer();

  ArenaGeometryBuffer(const ArenaGeometryBuffer&) = delete;
  ArenaGeometryBuffer(ArenaGeometryBuffer&&) = delete;
  auto operator=(const ArenaGeometryBuffer&) -> ArenaGeometryBuffer& = delete;
  auto operator=(ArenaGeometryBuffer&&) -> ArenaGeometryBuffer& = delete;

  auto addMesh(const IGeometryData& geometryData) -> MeshHandle;
  auto removeMesh(MeshHandle meshHandle);

  [[nodiscard]] auto getGpuBufferEntries(const std::vector<RenderMeshData>& meshDataList)
      -> std::vector<GpuBufferEntry>&;

  [[nodiscard]] auto getBuffers() const -> std::tuple<Buffer&, Buffer&>;

private:
  IBufferManager* bufferManager;

  std::unique_ptr<ArenaBuffer> vertexBuffer;
  std::unique_ptr<ArenaBuffer> indexBuffer;

  MapKey bufferKeygen{};
  std::unordered_map<size_t, BufferEntry> bufferEntries;

  std::vector<GpuBufferEntry> gpuBufferEntryCache;
};

}
