#pragma once

namespace tr {

struct IndirectPushConstants {
  uint64_t objectDataAddress;
  uint64_t cameraDataAddress;
  uint64_t objectDataIndexAddress;
  uint64_t animationDataAddress;
  uint32_t drawID;
  uint32_t objectCount;
};

}
