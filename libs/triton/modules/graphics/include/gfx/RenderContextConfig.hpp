#pragma once

namespace tr {
struct RenderContextConfig {
  bool useDescriptorBuffers{};
  uint32_t maxStaticObjects{};
  uint32_t maxDynamicObjects{};
  uint16_t maxTextures{};
  uint8_t framesInFlight{};
  float renderScale{1.f};
};
}
