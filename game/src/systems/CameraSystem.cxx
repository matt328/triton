#include "CameraSystem.hpp"
#include "components/Camera.hpp"
#include "components/Transform.hpp"

void CameraSystem::update(entt::registry& registry) {
   const auto view = registry.view<Transform, Camera>();

   for (auto [entity, transform, camera] : view.each()) {}
}