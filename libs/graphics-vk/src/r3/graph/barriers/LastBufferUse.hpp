#pragma once

#include "r3/ComponentIds.hpp"

namespace tr {

struct LastBufferUse {
  PassId passId;
  vk::AccessFlags2 accessMask;
  vk::PipelineStageFlags2 stageMask;
};

}
