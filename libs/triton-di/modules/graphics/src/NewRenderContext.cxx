#include "NewRenderContext.hpp"

namespace tr::gfx {
   NewRenderContext::NewRenderContext(std::shared_ptr<CommandBufferManager> newCommandBufferManager)
       : commandBufferManager{std::move(newCommandBufferManager)} {
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
