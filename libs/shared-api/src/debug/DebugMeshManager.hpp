#pragma once
#include "api/gw/Handles.hpp"

namespace tr {

class DebugMeshManager {
public:
  explicit DebugMeshManager();
  ~DebugMeshManager() = default;

  DebugMeshManager(const DebugMeshManager&) = default;
  DebugMeshManager(DebugMeshManager&&) = delete;
  auto operator=(const DebugMeshManager&) -> DebugMeshManager& = default;
  auto operator=(DebugMeshManager&&) -> DebugMeshManager& = delete;

  auto update() -> void;
  [[nodiscard]] auto getRenderableMeshes() const -> std::vector<MeshData>;

private:
  std::vector<MeshData> currentMeshData;
};

}
