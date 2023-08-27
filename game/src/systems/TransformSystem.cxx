#include "TransformSystem.hpp"
#include "components/Transform.hpp"

TransformSystem::TransformSystem() = default;

void TransformSystem::update(entt::registry& registry) const {
   const auto view = registry.view<Transform>();

   for (auto [entity, transform] : view.each()) {
      // this can be constant since our update timestep is fixed. what a time to be alive.
      transform.rotateByAxisAngle({0.f, 1.f, 0.f}, .25f);
   }
}

void TransformSystem::handleAction(const Actions::Action& action) {
   Core::Log::core->info("Transform System handling action {}", Actions::toString(action));
}
