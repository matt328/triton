#pragma once

#include "api/gfx/GeometryData.hpp"
#include "api/gw/editordata/GameObjectData.hpp"
namespace tr {

struct AddSkeleton {
  std::string name;
  std::string fileName;
  bool fromFile = false;
};

struct AddAnimation {
  std::string name;
  std::string fileName;
  bool fromFile = false;
};

struct AddModel {
  std::string name;
  std::string fileName;
  bool fromFile = false;
};

struct SelectEntity {
  std::optional<std::string> entityId;
};

struct AddStaticModel {
  std::string name;
  Orientation orientation;
  std::string modelName;
  bool fromFile = false;
  uint32_t count = 1;
};

struct AddStaticGeometry {
  std::string name;
  Orientation orientation;
  GeometryData geometryData;
  std::string entityName;
};

struct SaveProject {
  std::filesystem::path filePath;
};

struct LoadProject {
  std::filesystem::path filePath;
};

}
