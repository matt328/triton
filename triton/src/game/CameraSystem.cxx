#include "CameraSystem.hpp"
#include "Camera.hpp"
#include "Transform.hpp"

void CameraSystem::update(entt::registry& registry) {
   const auto view = registry.view<Transform, Camera>();

   for (auto [entity, transform, camera] : view.each()) {}
}