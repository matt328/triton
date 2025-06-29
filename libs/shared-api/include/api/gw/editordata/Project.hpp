#pragma once

#include "api/gw/editordata/FileAlias.hpp"
#include "api/gw/editordata/GameObjectData.hpp"

namespace tr {

struct Project {
  std::unordered_map<std::string, FileAlias> skeletons;
  std::unordered_map<std::string, FileAlias> animations;
  std::unordered_map<std::string, FileAlias> models;

  std::unordered_map<std::string, GameObjectData> gameObjects;

  template <class T>
  void serialize(T& archive) {
    archive(skeletons, animations, models, gameObjects);
  }
};

}
