#include "EditorSystem.hpp"
#include "api/gw/editordata/EditorState.hpp"

namespace tr {

auto EditorSystem::update(entt::registry& registry) -> const EditorContextData& {

  auto editorState = registry.ctx().get<EditorContextData>();

  return editorState;
}

}
