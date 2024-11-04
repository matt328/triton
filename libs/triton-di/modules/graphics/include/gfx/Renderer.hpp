#pragma once

#include "IRenderer.hpp"

namespace tr::gfx {
   class Renderer : public IRenderer {
    public:
      Renderer() = default;
      ~Renderer() = default;

      void render() override;
   };
}