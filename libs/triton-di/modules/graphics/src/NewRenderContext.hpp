#pragma once
#include <gfx/IRenderContext.hpp>

namespace tr::gfx {

class NewRenderContext final : public IRenderContext {
    public:
      ~NewRenderContext() override;
      void render() override;
      void waitIdle() override;
      void setRenderData(const cm::gpu::RenderData& renderData) override;
   };

}
