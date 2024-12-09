#include "NewRenderContext.hpp"

namespace tr::gfx {
   NewRenderContext::NewRenderContext(std::shared_ptr<task::IFrameManager> newFrameManager,
                                      std::shared_ptr<task::IRenderScheduler> newRenderScheduler)
       : frameManager{std::move(newFrameManager)}, renderScheduler{std::move(newRenderScheduler)} {
      Log.trace("Creating NewRenderContext");
   }

   NewRenderContext::~NewRenderContext() {
      Log.trace("Destroying NewRenderContext");
   }

   void NewRenderContext::renderNextFrame() {
      auto& frame = frameManager->acquireFrame();

      renderScheduler->prepareFrame(frame);

      // This will .begin() and .end() on the commandBuffers before handing them off to renderTasks
      renderScheduler->recordRenderTasks(frame);

      renderScheduler->endFrame(frame);

      // Command Buffers cannot be returned to the pool until its associated fence is signaled.
      frameManager->submitFrame(frame);
   }

   void NewRenderContext::waitIdle() {
      Log.trace("waitIdle");
   }

   void NewRenderContext::setRenderData(const cm::gpu::RenderData& renderData) {
      Log.trace("setRenderData");
   }
}
