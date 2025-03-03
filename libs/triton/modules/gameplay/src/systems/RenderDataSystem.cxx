#include "RenderDataSystem.hpp"

#include "cm/GpuCameraData.hpp"
#include "cm/RenderData.hpp"
#include "gp/components/Renderable.hpp"
#include "gp/components/Camera.hpp"
#include "gp/components/Resources.hpp"
#include "gp/components/Transform.hpp"
#include "gp/components/Animation.hpp"
#include "gp/components/TerrainDefinition.hpp"

namespace tr {

RenderDataSystem::RenderDataSystem(std::shared_ptr<EntityService> newEntityService)
    : entityService{std::move(newEntityService)} {};

auto RenderDataSystem::update(RenderData& renderData) -> void {

  const auto camFn = [](RenderData& renderData, const Camera& cam) {
    renderData.cameraData = GpuCameraData{.view = cam.view,
                                          .proj = cam.projection,
                                          .viewProj = cam.view * cam.projection};
  };

  const auto staticFn = [](RenderData& renderData,
                           entt::entity,
                           const Renderable& renderable,
                           const Transform& transform) {
    for (const auto& [meshHandle, topology, textureHandle] : renderable.meshData) {
      renderData.staticGpuMeshData.emplace_back(meshHandle, topology);
      renderData.objectData.emplace_back(transform.transformation, textureHandle);
    }
  };

  const auto terrainFn =
      [](RenderData& renderData, entt::entity, const TerrainDefinition& terrainDefinition) {
        renderData.terrainDefinition =
            GpuTerrainDefinition{.terrainSize = terrainDefinition.terrainSize};
      };

  uint32_t jointMatricesIndex = 0;
  const auto dynamicFn = [&jointMatricesIndex](RenderData& renderData,
                                               [[maybe_unused]] entt::entity entity,
                                               const Animation& animationData,
                                               const Renderable& renderable,
                                               const Transform& transform) {
    int64_t size = animationData.jointMatrices.size();
    TracyPlot("jointMatrices size", size);
    for (const auto jointMatrix : animationData.jointMatrices) {
      renderData.animationData.push_back({jointMatrix});
    }

    // Add everything to the RenderData struct
    for (const auto& [meshHandle, topology, textureHandle] : renderable.meshData) {
      renderData.dynamicMeshData.emplace_back(meshHandle, topology);
      renderData.dynamicObjectData.emplace_back(transform.transformation,
                                                textureHandle,
                                                jointMatricesIndex);
    }

    jointMatricesIndex += animationData.jointMatrices.size();
  };

  entityService->updateRenderData(camFn, staticFn, dynamicFn, terrainFn, renderData);
}

}
