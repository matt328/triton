#pragma once

namespace tr {
struct IndirectPushConstants {
  uint32_t drawID;      // Matches `uint drawID` in GLSL
  uint64_t baseAddress; // Matches `uint64_t baseAddress` in GLSL
  uint64_t cameraDataAddress;
};
}
