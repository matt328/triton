#include "systems/ScriptingSystem.hpp"
#include "components/Script.hpp"
#include "components/Renderable.hpp"
#include "components/Transform.hpp"

void ScriptingSystem::update(entt::registry& registry) {
   for (const auto view = registry.view<Script>(); const auto entity : view) {
      auto& script = view.get<Script>(entity);
   }
}