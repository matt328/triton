#include "EditorSystem.hpp"
#include "api/gw/editordata/EditorState.hpp"

namespace tr {

auto EditorSystem::update(entt::registry& registry) -> EditorState {
  auto contextData = registry.ctx().get<EditorContextData>();

  auto editorState = EditorState{
      .contextData = contextData,
      .objectDataMap = {},
  };

  return editorState;
}

}
