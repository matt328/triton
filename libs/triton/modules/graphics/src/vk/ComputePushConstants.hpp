#pragma once

namespace tr {

struct ComputePushConstants {
  uint64_t drawCommandBufferAddress;
  uint64_t gpuBufferEntryBufferAddress;
  uint64_t objectDataBufferAddress;
  uint64_t countBufferAddress;
  uint64_t objectDataIndexBufferAddress;
  uint32_t objectCount;
};

}
