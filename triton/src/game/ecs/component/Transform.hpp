#pragma once

#include "graphics/gui/EntityEditor.hpp"

namespace Triton::Game::Ecs {
   struct Transform {
      float xRot{}, yRot{}, zRot{};
      float x{}, y{}, z{};
   };
}

namespace MM {
   template <>
   void ComponentEditorWidget<Triton::Game::Ecs::Transform>(entt::registry& reg,
                                                            entt::registry::entity_type e);
}
