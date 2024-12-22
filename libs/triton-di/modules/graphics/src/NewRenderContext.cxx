#include "NewRenderContext.hpp"

namespace tr {
NewRenderContext::NewRenderContext(std::shared_ptr<IFrameManager> newFrameManager,
                                   std::shared_ptr<IRenderScheduler> newRenderScheduler,
                                   std::shared_ptr<queue::Graphics> newGraphicsQueue)
    : frameManager{std::move(newFrameManager)},
      renderScheduler{std::move(newRenderScheduler)},
      graphicsQueue{std::move(newGraphicsQueue)} {
   Log.trace("Creating NewRenderContext");
}

NewRenderContext::~NewRenderContext() {
   Log.trace("Destroying NewRenderContext");
}

void NewRenderContext::renderNextFrame() {
   const auto result = frameManager->acquireFrame();

   if (std::holds_alternative<std::reference_wrapper<Frame>>(result)) {
      const auto& frame = std::get<std::reference_wrapper<Frame>>(result);
      renderScheduler->setupCommandBuffersForFrame(frame);
      renderScheduler->recordRenderTasks(frame);
      renderScheduler->endFrame(frame);
      return;
   }

   if (const auto acquireResult = std::get<ImageAcquireResult>(result);
       acquireResult == ImageAcquireResult::Error) {
      Log.warn("Failed to acquire swapchain image");
   }
}

void NewRenderContext::waitIdle() {
   Log.trace("waitIdle");
}

void NewRenderContext::setRenderData([[maybe_unused]] const RenderData& renderData) {
   Log.trace("setRenderData");
}
}
