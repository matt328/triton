#pragma once

#include "r3/graph/BufferUsageInfo.hpp"
#include "r3/graph/ImageUsageInfo.hpp"

namespace tr {

struct PassGraphInfo {
  std::vector<ImageUsageInfo> imageWrites{};
  std::vector<ImageUsageInfo> imageReads{};

  std::vector<BufferUsageInfo> bufferWrites{};
  std::vector<BufferUsageInfo> bufferReads{};
};

}
