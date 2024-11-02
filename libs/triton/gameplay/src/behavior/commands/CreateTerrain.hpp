#pragma once

#include "behavior/CommandQueue.hpp"
#include "cm/EntitySystemTypes.hpp"
#include "cm/Handles.hpp"
#include "components/Renderable.hpp"
#include "components/Terrain.hpp"
#include "components/Transform.hpp"
#include "components/DebugConstants.hpp"

namespace tr::gp {

   class CreateTerrainCommand : public ICommand {
    private:
      std::vector<cm::ModelData> modelDataList;
      std::vector<cm::TerrainCreatedFn> callbacks;

    public:
      explicit CreateTerrainCommand(const std::vector<cm::ModelData>& modelData,
                                    const std::vector<cm::TerrainCreatedFn>& callbacks)
          : modelDataList{modelData}, callbacks{callbacks} {
      }

      void execute(entt::registry& registry) const override {
         const auto entity = registry.create();

         auto meshDatas = std::vector<cm::MeshData>{};
         for (const auto& handle : modelDataList) {
            meshDatas.push_back(handle.meshData);
         }

         registry.emplace<cmp::Renderable>(entity, meshDatas);
         registry.emplace<cmp::TerrainMarker>(entity);
         registry.emplace<cmp::Transform>(entity);

         const auto debugConstants = registry.create();
         registry.emplace<cmp::Transform>(debugConstants,
                                          glm::zero<glm::vec3>(),
                                          glm::vec3(200.f, 1000.f, 200.f));
         registry.emplace<cmp::DebugConstants>(debugConstants, 16.f);

         for (const auto& func : callbacks) {
            func(entity);
         }
      }
   };

}
