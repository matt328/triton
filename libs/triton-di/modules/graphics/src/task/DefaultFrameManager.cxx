#include "DefaultFrameManager.hpp"
#include <gfx/RenderContextConfig.hpp>

namespace tr::gfx::task {
   DefaultFrameManager::DefaultFrameManager(
       const RenderContextConfig& rendererConfig,
       std::shared_ptr<CommandBufferManager> newCommandBufferManager)
       : currentFrame{0}, commandBufferManager{std::move(newCommandBufferManager)} {

      for (uint8_t i = 0; i < rendererConfig.framesInFlight; ++i) {
         frames.push_back(std::make_unique<Frame>(frames.size()));
      }
   }

   DefaultFrameManager::~DefaultFrameManager() {
      Log.trace("Destroying DefaultFrameManager");
   }

   auto DefaultFrameManager::acquireFrame() -> Frame& {
      const auto& frame = frames[currentFrame];



      frame->acquireSwapchainImage();
      currentFrame = (currentFrame + 1) % frames.size();
      return *frame;
   }

   auto DefaultFrameManager::submitFrame(Frame& frame) -> void {
   }
}
