#include "RenderDataSystem.hpp"

#include "cm/ObjectData.hpp"
#include "cm/RenderData.hpp"
#include "gp/components/Renderable.hpp"
#include "gp/components/Camera.hpp"
#include "gp/components/Resources.hpp"
#include "gp/components/Transform.hpp"
#include "gp/components/Animation.hpp"
#include "gp/components/TerrainMarker.hpp"

namespace tr {

auto RenderDataSystem::update(entt::registry& registry, RenderData& renderData) -> void {

  if (!registry.ctx().contains<const CurrentCamera>()) {
    Log.trace("Context doesn't contain current camera");
    return;
  }

  const auto cameraEntity = registry.ctx().get<const CurrentCamera>();
  const auto cam = registry.get<Camera>(cameraEntity.currentCamera);

  renderData.cameraData =
      CameraData{.view = cam.view, .proj = cam.projection, .viewProj = cam.view * cam.projection};

  // Static Models and Terrain
  for (const auto view = registry.view<Renderable, Transform>(entt::exclude<Animation>);
       const auto& [entity, renderable, transform] : view.each()) {

    const auto isTerrainEntity = registry.any_of<TerrainMarker>(entity);

    for (const auto& [meshHandle, topology, textureHandle] : renderable.meshData) {
      const auto objectDataPosition = renderData.objectData.size();
      if (isTerrainEntity) {
        if (topology == Topology::Triangles) {
          renderData.terrainMeshData.emplace_back(meshHandle,
                                                  Topology::Triangles,
                                                  objectDataPosition);
        } else if (topology == Topology::LineList) {
          renderData.staticMeshData.emplace_back(meshHandle, topology, objectDataPosition);
          renderData.objectData.emplace_back(transform.transformation, textureHandle);
        }
      } else {
        renderData.staticGpuMeshData.emplace_back(meshHandle, topology, objectDataPosition);
        renderData.objectData.emplace_back(transform.transformation, textureHandle);
      }
    }
  }

  // Animated Models
  const auto animationsView = registry.view<Animation, Renderable, Transform>();
  uint32_t jointMatricesIndex = 0;
  for (const auto& [entity, animationData, renderable, transform] : animationsView.each()) {
    // Convert the jointMap into a list of join matrices the gpu needs
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
      const auto objectDataPosition = renderData.objectData.size();
      renderData.skinnedMeshData.emplace_back(meshHandle, topology, objectDataPosition);
      renderData.objectData.emplace_back(transform.transformation,
                                         textureHandle,
                                         jointMatricesIndex);
    }

    jointMatricesIndex += jointMatrices.size();
  }
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
