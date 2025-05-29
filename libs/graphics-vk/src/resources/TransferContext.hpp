#pragma once

#include "buffers/ManagedBuffer.hpp"
#include "resources/allocators/IBufferAllocator.hpp"

namespace tr {

struct TransferContext {
  Handle<ManagedBuffer> stagingBuffer;
  std::unique_ptr<IBufferAllocator> stagingAllocator;
};

}
