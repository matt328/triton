#pragma once
#include "cm/RenderData.hpp" // All types involved in di::bind must be fully defined.

namespace tr::gfx {

   class IRenderContext {
    public:
      explicit IRenderContext() = default;
      virtual ~IRenderContext() = default;

      IRenderContext(const IRenderContext&) = default;
      IRenderContext(IRenderContext&&) = delete;
      auto operator=(const IRenderContext&) -> IRenderContext& = default;
      auto operator=(IRenderContext&&) -> IRenderContext& = delete;

      virtual void renderNextFrame() = 0;
      virtual void waitIdle() = 0;

      virtual void setRenderData(const cm::gpu::RenderData& renderData) = 0;
   };
}
