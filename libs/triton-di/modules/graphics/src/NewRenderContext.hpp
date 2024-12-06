#pragma once
#include <gfx/IRenderContext.hpp>
#include <task/IFrameManager.hpp>

namespace tr::gfx {

   class NewRenderContext final : public IRenderContext {
    public:
      explicit NewRenderContext(std::shared_ptr<task::IFrameManager> newFrameManager);
      ~NewRenderContext() override;

      NewRenderContext(const NewRenderContext&) = delete;
      NewRenderContext(NewRenderContext&&) = delete;
      auto operator=(const NewRenderContext&) -> NewRenderContext& = delete;
      auto operator=(NewRenderContext&&) -> NewRenderContext& = delete;

      void renderNextFrame() override;
      void waitIdle() override;
      void setRenderData(const cm::gpu::RenderData& renderData) override;

    private:
      std::shared_ptr<task::IFrameManager> frameManager;
   };

}
