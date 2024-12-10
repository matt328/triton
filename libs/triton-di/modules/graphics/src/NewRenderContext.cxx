#include "NewRenderContext.hpp"

namespace tr::gfx {
   NewRenderContext::NewRenderContext(std::shared_ptr<task::IFrameManager> newFrameManager,
                                      std::shared_ptr<task::IRenderScheduler> newRenderScheduler,
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

         // The render context should probably be the thing interacting with the Queues, not the
         // frame manager
         const auto buffers = std::array{(**startBuffer), **staticCommandBuffer, **endBuffer};
         const auto submitInfo = vk::SubmitInfo{
             .commandBufferCount = buffers.size(),
             .pCommandBuffers = buffers.data(),
         };
         graphicsQueue->getQueue().submit(submitInfo, inFlightFence);
         return;
      }

      if (const auto acquireResult = std::get<ImageAcquireResult>(result);
          acquireResult == ImageAcquireResult::NeedsResize) {
         // resizeSwapchain();
      } else if (acquireResult == ImageAcquireResult::Error) {
         Log.warn("Failed to acquire swapchain image");
      }
   }

   void NewRenderContext::waitIdle() {
      Log.trace("waitIdle");
   }

   void NewRenderContext::setRenderData(const cm::gpu::RenderData& renderData) {
      Log.trace("setRenderData");
   }
}
