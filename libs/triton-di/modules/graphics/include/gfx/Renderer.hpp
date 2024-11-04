#pragma once

#include "IRenderer.hpp"

namespace tr::gfx {
   class Renderer : public IRenderer {
    public:
      explicit Renderer();
      ~Renderer() override = default;

      void render() override;
      void waitIdle() override;
   };
}