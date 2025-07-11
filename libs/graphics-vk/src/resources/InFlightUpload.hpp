#pragma once

#include "api/fx/ResourceEvents.hpp"

namespace tr {

using ResponseVariant = std::variant<StaticMeshUploaded, StaticModelUploaded, DynamicModelUploaded>;

struct InFlightUpload {

  static auto from(const StaticModelRequest& req, Handle<Geometry> geometryHandle)
      -> InFlightUpload {
    return {.requestId = req.requestId,
            .remainingComponents = 2,
            .responseEvent = StaticModelUploaded{.batchId = req.batchId,
                                                 .requestId = req.requestId,
                                                 .entityName = req.entityName,
                                                 .geometryHandle = geometryHandle}};
  }

  static auto from(const StaticMeshRequest& req) -> InFlightUpload {
    return {.requestId = req.requestId,
            .remainingComponents = 2,
            .responseEvent = StaticMeshUploaded{
                .batchId = req.batchId,
                .requestId = req.requestId,
                .entityName = req.entityName,
            }};
  }

  uint64_t requestId;
  size_t remainingComponents = 2;
  ResponseVariant responseEvent;
};

}
