#include "Frame.hpp"

namespace tr::gfx {

   Frame::Frame(const uint8_t newIndex) : index{newIndex} {
   }

   auto Frame::acquireSwapchainImage() -> void {
   }

   auto Frame::getIndex() const -> uint8_t {
      return index;
   }
}
