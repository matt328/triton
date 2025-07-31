#pragma once

#include "api/fx/ResourceEvents.hpp"

namespace tr {

using ResponseVariant = std::variant<StaticMeshUploaded, StaticModelUploaded, DynamicModelUploaded>;

enum class ComponentType : uint8_t {
  Geometry = 0,
  Image,
};

struct InFlightUpload {
  uint64_t requestId;
  std::set<ComponentType> remainingComponents;
  ResponseVariant responseEvent;
};

using InFlightUploadMap = std::unordered_map<uint64_t, InFlightUpload>;

}
