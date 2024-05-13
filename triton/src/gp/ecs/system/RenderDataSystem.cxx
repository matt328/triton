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

   /// Loops through the registry and fills in the given RenderData struct
   void update(const entt::registry& registry, gfx::RenderData& renderData) {

      const auto cameraEntity = registry.ctx().get<const CurrentCamera>();
      const auto cam = registry.get<Camera>(cameraEntity.currentCamera);

      renderData.cameraData = gfx::CameraData{cam.view, cam.projection, cam.view * cam.projection};

      const auto view = registry.view<Renderable, Transform>();
      for (auto [entity, renderable, transform] : view.each()) {

         const auto isTerrainEntity = registry.any_of<TerrainMarker>(entity);

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

      const auto animationsView = registry.view<Animation>();
      for (auto [entity, animationData] : animationsView.each()) {
         /*
            - Add joint matrices to an array in RenderData.
            - There will be a single buffer of animation data for all active animated models, and a
            JointMatrixHandle will index into this the same way the TextureHandle indexes into the
            texture buffer
            - Also need to add a dynamicMeshData that includes a MeshHandle, TextureHandle, and a
            JoinMatrixHandle
            - JointMatrixHandle will need an offset into jointMatrices array and maybe also a size?
            - When rendering, this JointMatrixHandle will need to be part of ObjectData
            - extend ObjectData to have a uint jointMatrixOffset, in the shader, use that to look up
            the model's first joint matrix from the buffer
            - Still need to understand if that's all that needs done is multiplying each vertex's
            transformation by the weighted joint matrix in the shader, and if the animation library
            takes care of everything else
         */
      }

      const auto debugView = registry.view<Transform, DebugConstants>();
      for (const auto [entity, transform, debugConstants] : debugView.each()) {
         renderData.pushConstants.lightPosition = glm::vec4(transform.position, 1.f);
         renderData.pushConstants.params = glm::vec4(debugConstants.specularPower, 0.f, 0.f, 0.f);
      }
   }
}
