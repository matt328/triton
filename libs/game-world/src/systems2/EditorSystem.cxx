#include "EditorSystem.hpp"
#include "api/gw/editordata/EditorState.hpp"

namespace tr {

auto EditorSystem::update(entt::registry& registry) -> EditorState {
  auto contextData = registry.ctx().get<EditorContextData>();

  auto editorState = EditorState{
      .contextData = contextData,
      .objectDataMap = {},
  };

  const auto view = registry.view<GameObjectData>();

  for (const auto& [entity, gameObjectData] : view.each()) {
    editorState.objectDataMap.emplace(gameObjectData.name, gameObjectData);
  }

  return editorState;
}

}
