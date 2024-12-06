#pragma once

namespace tr::gfx {

   class Frame {
    public:
      explicit Frame(uint8_t newIndex);
      auto acquireSwapchainImage() -> void;

      [[nodiscard]] auto getIndex() const -> uint8_t;

    private:
      uint8_t index;
   };

}
