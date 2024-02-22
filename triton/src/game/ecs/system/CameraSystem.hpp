#pragma once

namespace Triton::Actions {
   struct Action;
}

namespace Triton::Game::Ecs::CameraSystem {
   void fixedUpdate(entt::registry& registry);

   void handleAction(entt::registry& registry, const Triton::Actions::Action& action);

}
