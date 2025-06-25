#include "EditorSystem.hpp"

namespace tr {

auto EditorSystem::update(entt::registry& registry) -> const EditorState& {
  return registry.ctx().get<EditorState>();
}

}
