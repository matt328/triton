#pragma once

namespace tr {

struct IndirectPushConstants {
  uint32_t drawID;
  uint64_t objectDataAddress;
  uint64_t cameraDataAddress;
  uint32_t objectDataLength;
};

struct ComputePushConstants {
  uint64_t drawCommandBufferAddress;
  uint64_t gpuBufferEntryBufferAddress;
  uint64_t objectDataBufferAddress;
  uint32_t instanceDataLength;
};

}
