#pragma once

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

}
