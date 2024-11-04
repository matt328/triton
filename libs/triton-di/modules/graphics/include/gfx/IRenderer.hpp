#pragma once

namespace tr::gfx {
   class IRenderer {
    public:
      IRenderer() = default;
      virtual ~IRenderer() = default;

      virtual void render() = 0;
   };
}