#pragma once

#include "api/gw/GameObjectType.hpp"
#include "api/gw/editordata/FileAlias.hpp"
#include "api/gw/editordata/GameObjectData.hpp"

namespace tr {

struct EditorContextData {

  struct Assets {
    std::unordered_map<std::string, FileAlias> skeletons;
    std::unordered_map<std::string, FileAlias> animations;
    std::unordered_map<std::string, FileAlias> models;
  };

  struct Scene {
    std::unordered_map<std::string, tr::GameObjectId> objectNameMap;
  };

  std::optional<std::string> selectedEntity = std::nullopt;
  bool saved = true;
  Assets assets{};
  Scene scene{};
};

struct EditorState {
  EditorContextData contextData;
  std::unordered_map<std::string, GameObjectData> objectDataMap;
};

}
