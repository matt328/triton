#include "NewRenderContext.hpp"

namespace tr::gfx {
   NewRenderContext::NewRenderContext(std::shared_ptr<CommandBufferManager> newCommandBufferManager)
       : commandBufferManager{std::move(newCommandBufferManager)} {
      Log.trace("Creating NewRenderContext");
   }

   NewRenderContext::~NewRenderContext() {
      Log.trace("Destroying NewRenderContext");
   }

   void NewRenderContext::renderNextFrame() {
      // AcquireFrame will get command buffers from commandbuffermanager and place them in the Frame
      auto& frame = frameManager->acquireFrame();

      // This will .begin() and .end() on the commandBuffers before handing them off to renderTasks
      renderScheduler->recordRenderTasks(frame);

      // Command Buffers cannot be returned to the pool until its associated fence is signaled.
      frameManager->submitFrame();
   }

   void NewRenderContext::waitIdle() {
      Log.trace("waitIdle");
   }

   void NewRenderContext::setRenderData(const cm::gpu::RenderData& renderData) {
      Log.trace("setRenderData");
   }
}
