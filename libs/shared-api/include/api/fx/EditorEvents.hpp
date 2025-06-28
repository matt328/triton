#pragma once

#include "api/gw/editordata/GameObjectData.hpp"
namespace tr {

struct AddSkeleton {
  std::string name;
  std::string fileName;
};

struct AddAnimation {
  std::string name;
  std::string fileName;
};

struct AddModel {
  std::string name;
  std::string fileName;
};

struct SelectEntity {
  std::optional<std::string> entityId;
};

struct AddStaticModel {
  std::string name;
  Orientation orientation;
  std::string modelName;
};

}
