#pragma once
#include <gfx/IRenderContext.hpp>

namespace tr::gfx {

   class NewRenderContext final : public IRenderContext {
    public:
      NewRenderContext();
      ~NewRenderContext() override;

      NewRenderContext(const NewRenderContext&) = delete;
      NewRenderContext(NewRenderContext&&) = delete;
      auto operator=(const NewRenderContext&) -> NewRenderContext& = delete;
      auto operator=(NewRenderContext&&) -> NewRenderContext& = delete;

      void render() override;
      void waitIdle() override;
      void setRenderData(const cm::gpu::RenderData& renderData) override;
   };

}
