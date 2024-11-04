#pragma once

namespace tr::gfx {
   class IRenderer {
    public:
      explicit IRenderer() = default;
      virtual ~IRenderer() = default;

      virtual void render() = 0;
      virtual void waitIdle() = 0;
   };
}