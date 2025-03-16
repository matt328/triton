#pragma once
#include "cm/Handles.hpp"

namespace tr {
class IDebugService;

class DebugMeshManager {
public:
  explicit DebugMeshManager(std::shared_ptr<IDebugService> newDebugService);
  ~DebugMeshManager() = default;

  DebugMeshManager(const DebugMeshManager&) = default;
  DebugMeshManager(DebugMeshManager&&) = delete;
  auto operator=(const DebugMeshManager&) -> DebugMeshManager& = default;
  auto operator=(DebugMeshManager&&) -> DebugMeshManager& = delete;

  auto update() -> void;
  [[nodiscard]] auto getRenderableMeshes() const -> std::vector<MeshData>;

private:
  std::shared_ptr<IDebugService> debugService;

  std::vector<MeshData> currentMeshData;
};

}
