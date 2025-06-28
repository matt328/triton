#pragma once

#include "api/gw/editordata/GameObjectData.hpp"

namespace tr {
struct EditorInfo {
  std::string name;
  Orientation initialOrientation;
  std::string modelName;
  std::string skeletonName;
  std::vector<std::string> animationNames;
};
}
