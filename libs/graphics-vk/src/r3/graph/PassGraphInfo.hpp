#pragma once

#include "r3/graph/ImageUsageInfo.hpp"

namespace tr {
struct PassGraphInfo {
  std::vector<ImageUsageInfo> writes;
  std::vector<ImageUsageInfo> reads;
};
}
