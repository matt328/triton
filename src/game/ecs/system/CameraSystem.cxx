#include "CameraSystem.hpp"

#include "game/ecs/component/Camera.hpp"

namespace Triton::Game::Ecs {
   void update(entt::registry& registry) {
      const auto view = registry.view<Camera>();
      for (auto [entity, cam] : view.each()) {}
   }
}