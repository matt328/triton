#include "RenderDataSystem.hpp"

#include "cm/GpuCameraData.hpp"
#include "cm/RenderData.hpp"
#include "gp/components/Renderable.hpp"
#include "gp/components/Camera.hpp"
#include "gp/components/Resources.hpp"
#include "gp/components/Transform.hpp"
#include "gp/components/Animation.hpp"
#include "gp/components/TerrainMarker.hpp"

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
                           bool isTerrain,
                           entt::entity,
                           const Renderable& renderable,
                           const Transform& transform) {
    for (const auto& [meshHandle, topology, textureHandle] : renderable.meshData) {
      if (isTerrain) {
        if (topology == Topology::Triangles) {
          renderData.terrainMeshData.emplace_back(meshHandle, Topology::Triangles);
        } else if (topology == Topology::LineList) {
          renderData.staticMeshData.emplace_back(meshHandle, topology);
          renderData.objectData.emplace_back(transform.transformation, textureHandle);
        }
      } else {
        // TODO(matt) figure out a better name for MeshData and RenderMeshData
        renderData.staticGpuMeshData.emplace_back(meshHandle, topology);
        renderData.objectData.emplace_back(transform.transformation, textureHandle);
      }
    }
  };

  uint32_t jointMatricesIndex = 0;

  const auto dynamicFn = [&jointMatricesIndex, this](RenderData& renderData,
                                                     [[maybe_unused]] entt::entity entity,
                                                     const Animation& animationData,
                                                     const Renderable& renderable,
                                                     const Transform& transform) {
    // Convert the jointMap into a list of join matrices the gpu needs
    if (animationData.models.empty()) {
      Log.trace("Models are empty, skipping for now");
      return;
    }
    auto jointMatrices = std::vector<glm::mat4>{};
    jointMatrices.resize(animationData.jointMap.size());
    int index = 0;
    for (const auto& [position, jointId] : animationData.jointMap) {
      auto inverseBindMatrix = animationData.inverseBindMatrices[index];
      if (animationData.renderBindPose) {
        inverseBindMatrix = glm::identity<glm::mat4>();
      }
      jointMatrices[position] = convertOzzToGlm(animationData.models[jointId]) * inverseBindMatrix;
      ++index;
    }

    for (const auto jointMatrix : jointMatrices) {
      renderData.animationData.push_back({jointMatrix});
    }

    // Add everything to the RenderData struct
    for (const auto& [meshHandle, topology, textureHandle] : renderable.meshData) {
      renderData.dynamicMeshData.emplace_back(meshHandle, topology);
      renderData.dynamicObjectData.emplace_back(transform.transformation,
                                                textureHandle,
                                                jointMatricesIndex);
    }

    jointMatricesIndex += jointMatrices.size();
  };

  entityService->updateRenderDataCamera(camFn, staticFn, dynamicFn, renderData);
}

auto RenderDataSystem::convertOzzToGlm(const ozz::math::Float4x4& ozzMatrix) -> glm::mat4 {
  glm::mat4 glmMatrix{};

  for (int i = 0; i < 4; ++i) {
    std::array<float, 4> temp{};
    ozz::math::StorePtrU(ozzMatrix.cols[i], temp.data());
    for (int j = 0; j < 4; ++j) {
      glmMatrix[i][j] = temp[j];
    }
  }
  return glmMatrix;
}
}
