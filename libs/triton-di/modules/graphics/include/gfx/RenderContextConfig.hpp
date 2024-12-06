#pragma once

namespace tr::gfx {
   struct RenderContextConfig {
      bool useDescriptorBuffers{};
      uint16_t maxTextures{};
      uint8_t framesInFlight{};
   };
}
