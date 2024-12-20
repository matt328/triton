#pragma once

#include "../CommandQueue.hpp"
#include <entt/entity/fwd.hpp>

#include "gp/components/EditorInfo.hpp"
#include "gp/components/Camera.hpp"
#include "gp/components/Transform.hpp"
#include "gfx/ResourceManager.hpp"

namespace tr::gp {
   class CreateTestEntityCommand final
       : public ICommand<entt::registry&, const std::shared_ptr<gfx::ResourceManager>&> {
    public:
      explicit CreateTestEntityCommand(const std::string_view newName) : name{newName.data()} {
      }

      void execute(entt::registry& registry,
                   [[maybe_unused]] const std::shared_ptr<gfx::ResourceManager>& resourceManager)
          const override {
         const auto entity = registry.create();
         registry.emplace<cmp::EditorInfo>(entity, name);
         registry.emplace<cmp::Transform>(entity);
      }

    private:
      std::string name;
   };
}
