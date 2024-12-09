#include "DefaultFrameManager.hpp"
#include <gfx/RenderContextConfig.hpp>
#include "Frame.hpp"

namespace tr::gfx::task {
   DefaultFrameManager::DefaultFrameManager(
       const RenderContextConfig& rendererConfig,
       std::shared_ptr<CommandBufferManager> newCommandBufferManager,
       std::shared_ptr<VkResourceManager> newResourceManager,
       std::shared_ptr<queue::Graphics> newGraphicsQueue)
       : currentFrame{0},
         commandBufferManager{std::move(newCommandBufferManager)},
         resourceManager{std::move(newResourceManager)},
         graphicsQueue{std::move(newGraphicsQueue)} {

      for (uint8_t i = 0; i < rendererConfig.framesInFlight; ++i) {
         frames.push_back(
             std::make_unique<Frame>(frames.size(), commandBufferManager, resourceManager));
      }
   }

   DefaultFrameManager::~DefaultFrameManager() {
      Log.trace("Destroying DefaultFrameManager");
      frames.clear();
   }

   auto DefaultFrameManager::acquireFrame() -> Frame& {
      const auto& frame = frames[currentFrame];

      frame->beginFrame();

      currentFrame = (currentFrame + 1) % frames.size();
      return *frame;
   }

   auto DefaultFrameManager::submitFrame(Frame& frame) -> void {
      frame.endFrame();
   }
}
