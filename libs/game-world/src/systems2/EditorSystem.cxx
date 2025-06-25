#include "EditorSystem.hpp"

namespace tr {

auto EditorSystem::update(entt::registry& registry) -> EditorState {
  EditorState e{};
  e.assets.animations.emplace_back("some animation");
  return e;
}

}
