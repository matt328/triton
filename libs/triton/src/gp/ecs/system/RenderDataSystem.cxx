#include "RenderDataSystem.hpp"

#include "gfx/ObjectData.hpp"
#include "gfx/RenderData.hpp"
#include "gp/ecs/component/Animation.hpp"
#include "gp/ecs/component/Camera.hpp"
#include "gp/ecs/component/DebugConstants.hpp"
#include "gp/ecs/component/Renderable.hpp"
#include "gp/ecs/component/Resources.hpp"
#include "gp/ecs/component/Terrain.hpp"
#include "gp/ecs/component/Transform.hpp"

namespace tr::gp::ecs::RenderDataSystem {

   glm::mat4 convertOzzToGlm(const ozz::math::Float4x4& ozzMatrix) {
      glm::mat4 glmMatrix{};

      alignas(16) float temp[4];

      for (int i = 0; i < 4; ++i) {
         ozz::math::StorePtrU(ozzMatrix.cols[i], temp);
         for (int j = 0; j < 4; ++j) {
            glmMatrix[i][j] = temp[j];
         }
      }
      return glmMatrix;
   }

   /// Loops through the registry and fills in the given RenderData struct
   void update(const entt::registry& registry, gfx::RenderData& renderData) {

      const auto cameraEntity = registry.ctx().get<const CurrentCamera>();
      const auto cam = registry.get<Camera>(cameraEntity.currentCamera);

      renderData.cameraData = gfx::CameraData{cam.view, cam.projection, cam.view * cam.projection};

      const auto view = registry.view<Renderable, Transform>(entt::exclude<Animation>);
      for (auto [entity, renderable, transform] : view.each()) {

         const auto isTerrainEntity = registry.any_of<TerrainMarker>(entity);

         /*
            TODO: Make RenderData be more concrete classes so they're easier to reason about than
            a bunch of maps of long long
            Sizes are known up front so RenderData could be modeled as a SoA
         */

         for (auto& it : renderable.meshes) {
            const auto pos = renderData.objectData.size();
            if (isTerrainEntity) {
               renderData.terrainMeshData.emplace_back(it.first, pos);
            } else {
               renderData.staticMeshData.emplace_back(it.first, pos);
            }
            renderData.objectData.emplace_back(transform.transformation, it.second);
         }
      }

      const auto animationsView = registry.view<Animation, Renderable, Transform>();
      uint32_t jointMatricesIndex = 0;
      for (auto [entity, animationData, renderable, transform] : animationsView.each()) {
         auto jointMatrices = std::vector<glm::mat4>{};
         for (const auto& m : animationData.models) {
            jointMatrices.push_back(convertOzzToGlm(m));
         }

         renderData.animationData.insert(renderData.animationData.begin(),
                                         jointMatrices.begin(),
                                         jointMatrices.end());

         for (auto& it : renderable.meshes) {
            const auto pos = renderData.objectData.size();
            renderData.skinnedMeshData.emplace_back(it.first, pos);
            renderData.objectData.emplace_back(transform.transformation,
                                               it.second,
                                               jointMatricesIndex);
         }
         jointMatricesIndex += jointMatrices.size();
      }

      const auto debugView = registry.view<Transform, DebugConstants>();
      for (const auto [entity, transform, debugConstants] : debugView.each()) {
         renderData.pushConstants.lightPosition = glm::vec4(transform.position, 1.f);
         renderData.pushConstants.params = glm::vec4(debugConstants.specularPower, 0.f, 0.f, 0.f);
      }
   }
}
