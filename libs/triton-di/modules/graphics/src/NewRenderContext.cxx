#include "NewRenderContext.hpp"

namespace tr::gfx {
   NewRenderContext::~NewRenderContext() {
      Log.trace("Destroying NewRenderContext");
   }

   void NewRenderContext::render() {
   }

   void NewRenderContext::waitIdle() {
   }

   void NewRenderContext::setRenderData(const cm::gpu::RenderData& renderData) {
   }

}
