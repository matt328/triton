#include "RenderDataSystem.hpp"

#include "cm/ObjectData.hpp"
#include "cm/RenderData.hpp"

#include "components/Animation.hpp"
#include "components/Camera.hpp"
#include "components/DebugConstants.hpp"
#include "components/Renderable.hpp"
#include "components/Resources.hpp"
#include "components/Terrain.hpp"
#include "components/Transform.hpp"

namespace tr::gp::sys::RenderDataSystem {

   glm::mat4 convertOzzToGlm(const ozz::math::Float4x4& ozzMatrix) {
      glm::mat4 glmMatrix{};

      for (int i = 0; i < 4; ++i) {
         float temp[4];
         ozz::math::StorePtrU(ozzMatrix.cols[i], temp);
         for (int j = 0; j < 4; ++j) {
            glmMatrix[i][j] = temp[j];
         }
      }
      return glmMatrix;
   }

   /// Loops through the registry and fills in the given RenderData struct
   void update(const entt::registry& registry, cm::gpu::RenderData& renderData) {

      const auto cameraEntity = registry.ctx().get<const cmp::CurrentCamera>();
      const auto cam = registry.get<cmp::Camera>(cameraEntity.currentCamera);

      renderData.cameraData =
          cm::gpu::CameraData{cam.view, cam.projection, cam.view * cam.projection};

      for (const auto view =
               registry.view<cmp::Renderable, cmp::Transform>(entt::exclude<cmp::Animation>);
           const auto& [entity, renderable, transform] : view.each()) {

         const auto isTerrainEntity = registry.any_of<cmp::TerrainMarker>(entity);

         for (const auto& [meshHandle, textureHandle] : renderable.meshes) {
            const auto pos = renderData.objectData.size();
            if (isTerrainEntity) {
               renderData.terrainMeshData.emplace_back(meshHandle, pos);
            } else {
               renderData.staticMeshData.emplace_back(meshHandle, pos);
            }
            renderData.objectData.emplace_back(transform.transformation, textureHandle);
         }
      }

      const auto animationsView = registry.view<cmp::Animation, cmp::Renderable, cmp::Transform>();
      uint32_t jointMatricesIndex = 0;
      for (const auto& [entity, animationData, renderable, transform] : animationsView.each()) {
         auto jointMatrices = std::vector<glm::mat4>{};

         // Jointmap only has 25 things in it while animationData.models has 27

         jointMatrices.resize(animationData.jointMap.size());
         int i = 0;
         for (const auto [position, jointId] : animationData.jointMap) {
            auto inverseBindMatrix = animationData.inverseBindMatrices[i];
            if (animationData.renderBindPose) {
               inverseBindMatrix = glm::identity<glm::mat4>();
            }
            jointMatrices[position] =
                convertOzzToGlm(animationData.models[jointId]) * inverseBindMatrix;
            ++i;
         }

         for (const auto jointMatrix : jointMatrices) {
            renderData.animationData.push_back({jointMatrix});
         }

         // renderData.animationData.insert(renderData.animationData.begin(),
         //                                 jointMatrices.begin(),
         //                                 jointMatrices.end());

         for (const auto& [meshHandle, textureHandle] : renderable.meshes) {
            const auto pos = renderData.objectData.size();
            renderData.skinnedMeshData.emplace_back(meshHandle, pos);
            renderData.objectData.emplace_back(transform.transformation,
                                               textureHandle,
                                               jointMatricesIndex);
         }

         jointMatricesIndex += jointMatrices.size();
      }

      for (const auto debugView = registry.view<cmp::Transform, cmp::DebugConstants>();
           const auto [entity, transform, debugConstants] : debugView.each()) {
         renderData.pushConstants.lightPosition = glm::vec4(transform.position, 1.f);
         renderData.pushConstants.params = glm::vec4(debugConstants.specularPower, 0.f, 0.f, 0.f);
      }
   }
}
