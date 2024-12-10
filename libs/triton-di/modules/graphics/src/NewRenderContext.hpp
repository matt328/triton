#pragma once
#include <gfx/IRenderContext.hpp>
#include <gfx/IRenderScheduler.hpp>
#include <task/IFrameManager.hpp>

namespace tr::gfx {

   class NewRenderContext final : public IRenderContext {
    public:
      NewRenderContext(std::shared_ptr<task::IFrameManager> newFrameManager,
                       std::shared_ptr<task::IRenderScheduler> newRenderScheduler,
                       std::shared_ptr<queue::Graphics> newGraphicsQueue);
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
      std::shared_ptr<task::IRenderScheduler> renderScheduler;
      std::shared_ptr<queue::Graphics> graphicsQueue;
   };

}
