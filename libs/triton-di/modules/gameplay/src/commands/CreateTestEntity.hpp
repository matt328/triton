#pragma once

#include "../CommandQueue.hpp"
#include <entt/entity/fwd.hpp>

#include "gp/components/EditorInfo.hpp"
#include "gp/components/Camera.hpp"
#include "gp/components/Transform.hpp"

namespace tr::gp {
   class CreateTestEntityCommand : public ICommand<entt::registry> {
    public:
      explicit CreateTestEntityCommand(std::string_view newName) : name{newName.data()} {
      }

      void execute(entt::registry& registry) const override {
         const auto entity = registry.create();
         registry.emplace<cmp::EditorInfo>(entity, name);
         registry.emplace<cmp::Camera>(entity);
         registry.emplace<cmp::Transform>(entity);
      }

    private:
      std::string name;
   };
}
