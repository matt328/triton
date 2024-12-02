#include "NewRenderContext.hpp"

/*
 * RenderContext should have RenderScheduler injected into it, and configure it after its been
 * created.
 *
 */

namespace tr::gfx {
   NewRenderContext::NewRenderContext() {
      Log.trace("Creating NewRenderContext");
   }

   NewRenderContext::~NewRenderContext() {
      Log.trace("Destroying NewRenderContext");
   }

   void NewRenderContext::render() {
   }

   void NewRenderContext::waitIdle() {
      Log.trace("waitIdle");
   }

   void NewRenderContext::setRenderData(const cm::gpu::RenderData& renderData) {
      Log.trace("setRenderData");
   }

}
