#pragma once

#include "buffers/ImageUploadPlan.hpp"
#include "buffers/UploadPlan.hpp"
#include "resources/InFlightUpload.hpp"

namespace tr {
struct ProcessingResult {
  std::vector<UploadData> geometryUploads;
  std::optional<Handle<Geometry>> geometryHandle;

  std::vector<ImageUploadData> imageUploads;

  uint64_t requestId;
  ResponseVariant responseEvent;
};

}
