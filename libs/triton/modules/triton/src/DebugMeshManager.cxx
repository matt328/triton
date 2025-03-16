#include "DebugMeshManager.hpp"
#include "tr/IDebugService.hpp"
#include "as/ColorVertex.hpp"

namespace tr {

DebugMeshManager::DebugMeshManager(std::shared_ptr<IDebugService> newDebugService)
    : debugService{std::move(newDebugService)} {
}

auto DebugMeshManager::update() -> void {
  if (!debugService->isDirty()) {
    return;
  }
  // This has an inefficiency in that any time a debug gizmo is added or removed, it re uploades the
  // entire buffer. This really embraces the idea of immediate mode, but the API is abstracted away
  // from the client so that in here we could make this more efficient by tracking what shapes are
  // uploaded and what ones are not and use an arena buffer, but we'll see if it ever comes to that.
  // For now just use a large enough non-arena buffer, or perhaps even a cpu accessible buffer.
  auto vertexGroups = std::vector<std::vector<as::ColorVertex>>{};
  for (const auto& shape : debugService->getShapes()) {
    auto vertices = std::vector<as::ColorVertex>{};
    for (const auto& point : shape->getPoints()) {
      vertices.push_back(as::ColorVertex{.position = point, .color = shape->color.rgb});
    }
    vertexGroups.push_back(vertices);
  }

  // This needs to happen on another thread.
  const auto task = [this, vertexGroups]() -> std::vector<MeshHandle> {
    // This should block until everything is uploaded to the GPU
    return resourceManager->uploadLineData(vertexGroups);
  };

  const auto onComplete = [this](const std::vector<MeshHandle>& handles) {
    // This executes on the main thread (the same thread as the renderer is reading from them) so
    // no lock is needed. This also ensures the GPU won't be told to render anything that hasn't
    // finished uploading yet
    currentMeshData.clear();
    for (const auto& meshHandle : handles) {
      currentMeshData.emplace_back(meshHandle, Topology::LineList, 0L);
    }
  };

  taskQueue->enqueue(task, onComplete);
}

auto DebugMeshManager::getRenderableMeshes() const -> std::vector<MeshData> {
  return currentMeshData;
}

}
