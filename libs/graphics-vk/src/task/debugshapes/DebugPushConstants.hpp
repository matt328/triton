#pragma once

namespace tr {

struct DebugPushConstants {
  uint32_t drawID;
  uint64_t objectDataAddress;
  uint64_t cameraDataAddress;
  uint64_t objectDataIndexAddress;
  uint32_t objectCount;
};

}
