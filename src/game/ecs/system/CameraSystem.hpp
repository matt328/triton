#pragma once

#include "game/actions/Action.hpp"

namespace Triton::Game::Ecs::CameraSystem {
   void fixedUpdate(entt::registry& registry);

   void handleAction(entt::registry& registry, const Actions::Action& action);

}
