#pragma once

#include "api/fx/ResourceEvents.hpp"

namespace tr {

using ResponseVariant = std::variant<StaticMeshUploaded, StaticModelUploaded, DynamicModelUploaded>;

struct InFlightUpload {
  uint64_t requestId;
  size_t remainingComponents = 2;
  ResponseVariant responseEvent;
};

using InFlightUploadMap = std::unordered_map<uint64_t, InFlightUpload>;

}
