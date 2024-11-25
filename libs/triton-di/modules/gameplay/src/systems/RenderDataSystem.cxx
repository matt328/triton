#include "RenderDataSystem.hpp"

#include "cm/ObjectData.hpp"
#include "cm/RenderData.hpp"
#include "gp/components/Renderable.hpp"
#include "gp/components/Camera.hpp"
#include "gp/components/Transform.hpp"
#include "gp/components/Resources.hpp"
#include "gp/components/Animation.hpp"
#include "gp/components/TerrainMarker.hpp"

namespace tr::gp::sys {
   RenderDataSystem::RenderDataSystem(std::shared_ptr<Registry> newRegistry)
       : registry{std::move(newRegistry)} {
   }
   auto RenderDataSystem::update(cm::gpu::RenderData& renderData) const -> void {
      auto& reg = registry->getRegistry();

      if (!reg.ctx().contains<const cmp::CurrentCamera>()) {
         Log.trace("Context doesn't contain current camera");
         return;
      }

      const auto cameraEntity = reg.ctx().get<const cmp::CurrentCamera>();
      const auto cam = reg.get<cmp::Camera>(cameraEntity.currentCamera);

      renderData.cameraData = cm::gpu::CameraData{.view = cam.view,
                                                  .proj = cam.projection,
                                                  .viewProj = cam.view * cam.projection};

      // Static Models and Terrain
      for (const auto view =
               reg.view<cmp::Renderable, cmp::Transform>(entt::exclude<cmp::Animation>);
           const auto& [entity, renderable, transform] : view.each()) {

         const auto isTerrainEntity = reg.any_of<cmp::TerrainMarker>(entity);

         for (const auto& [meshHandle, topology, textureHandle] : renderable.meshData) {
            const auto objectDataPosition = renderData.objectData.size();
            if (isTerrainEntity) {
               if (topology == cm::Topology::Triangles) {
                  renderData.terrainMeshData.emplace_back(meshHandle,
                                                          cm::Topology::Triangles,
                                                          objectDataPosition);
               } else if (topology == cm::Topology::LineList) {
                  renderData.staticMeshData.emplace_back(meshHandle, topology, objectDataPosition);
                  renderData.objectData.emplace_back(transform.transformation, textureHandle);
               }
            } else {
               renderData.staticMeshData.emplace_back(meshHandle, topology, objectDataPosition);
               renderData.objectData.emplace_back(transform.transformation, textureHandle);
            }
         }
      }
   }
}
