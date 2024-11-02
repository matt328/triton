#pragma once

#include "../CommandQueue.hpp"
#include "../../components/Transform.hpp"

#include <entt/entity/fwd.hpp>

namespace tr::gp {
   class MoveCommand : public ICommand {
      entt::entity entity;
      glm::vec3 direction;

    public:
      MoveCommand(entt::entity entity, glm::vec3& direction)
          : entity{entity}, direction{direction} {
      }

      void execute(entt::registry& registry) const override {
         if (registry.valid(entity) && registry.all_of<cmp::Transform>(entity)) {
            auto& transform = registry.get<cmp::Transform>(entity);
            transform.position += direction;
         }
      }
   };
}
