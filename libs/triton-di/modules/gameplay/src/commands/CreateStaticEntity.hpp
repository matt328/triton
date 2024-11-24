#pragma once

#include "../CommandQueue.hpp"
#include "gfx/ResourceManager.hpp"
#include "gp/components/Transform.hpp"
#include "gp/components/Renderable.hpp"
#include "gp/components/EditorInfo.hpp"

namespace tr::gp::cmd {
   class CreateStaticEntityCommand final
       : public ICommand<entt::registry&, const std::shared_ptr<gfx::ResourceManager>&> {
    public:
      explicit CreateStaticEntityCommand(const std::string_view newModelFilename,
                                         const std::string_view newEntityName)
          : entityName{newEntityName.data()}, modelFilename{newModelFilename.data()} {
      }

      void execute(entt::registry& registry,
                   const std::shared_ptr<gfx::ResourceManager>& resourceManager) const override {
         const auto modelData = resourceManager->createModel(std::filesystem::path{modelFilename});
         const auto entity = registry.create();
         registry.emplace<cmp::Renderable>(entity, std::vector{modelData.meshData});
         registry.emplace<cmp::Transform>(entity);
         registry.emplace<cmp::EditorInfo>(entity, entityName);
      }

    private:
      std::string entityName;
      std::string modelFilename;
   };
}
