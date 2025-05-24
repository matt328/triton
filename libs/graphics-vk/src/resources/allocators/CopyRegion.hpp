#pragma once

#include "buffers/ManagedBuffer.hpp"

namespace tr {

struct CopyRegion {
  size_t destinationOffset;
  size_t size;
  void* srcData;
  Handle<ManagedBuffer> dstBuffer;
};

}
