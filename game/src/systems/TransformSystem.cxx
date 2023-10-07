#include "TransformSystem.hpp"
#include "components/Transform.hpp"
#include "Logger.hpp"

TransformSystem::TransformSystem() = default;

void TransformSystem::update(entt::registry& registry) const {
   const auto view = registry.view<Transform>();

   for (auto [entity, transform] : view.each()) {
      // this can be constant since our update timestep is fixed. what a time to be alive.
      transform.rotateByAxisAngle({0.f, 1.f, 0.f}, .25f);
   }
}
