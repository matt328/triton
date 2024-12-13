#include "DefaultFrameManager.hpp"
#include <gfx/RenderContextConfig.hpp>
#include "Frame.hpp"
#include "Maths.hpp"

#include <vk/Swapchain.hpp>

namespace tr::gfx::task {
   DefaultFrameManager::DefaultFrameManager(
       const RenderContextConfig& rendererConfig,
       std::shared_ptr<CommandBufferManager> newCommandBufferManager,
       std::shared_ptr<Device> newDevice,
       std::shared_ptr<Swapchain> newSwapchain,
       std::shared_ptr<VkResourceManager> newResourceManager,
       const std::shared_ptr<IDebugManager>& debugManager)
       : currentFrame{0},
         commandBufferManager{std::move(newCommandBufferManager)},
         device{std::move(newDevice)},
         swapchain{std::move(newSwapchain)},
         resourceManager{std::move(newResourceManager)} {

      debugManager->setDevice(device);

      for (uint8_t i = 0; i < rendererConfig.framesInFlight; ++i) {

         auto fence = device->getVkDevice().createFence(
             vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});

         auto acquireImageSemaphore = device->getVkDevice().createSemaphore({});
         debugManager->setObjectName(*acquireImageSemaphore,
                                     "AcquireImageSemaphoreFrame:" + std::to_string(i));

         auto renderFinishedSemaphore = device->getVkDevice().createSemaphore({});

         frames.push_back(std::make_unique<Frame>(static_cast<uint8_t>(frames.size()),
                                                  std::move(fence),
                                                  std::move(acquireImageSemaphore),
                                                  std::move(renderFinishedSemaphore)));

         const auto drawImageExtent =
             vk::Extent2D{.width = maths::scaleNumber(swapchain->getImageExtent().width,
                                                      rendererConfig.renderScale),
                          .height = maths::scaleNumber(swapchain->getImageExtent().height,
                                                       rendererConfig.renderScale)};
         resourceManager->createDrawImageAndView(frames[frames.size() - 1]->getDrawImageId(),
                                                 drawImageExtent);
      }
   }

   DefaultFrameManager::~DefaultFrameManager() {
      Log.trace("Destroying DefaultFrameManager");
      device->waitIdle();
      frames.clear();
   }

   auto DefaultFrameManager::acquireFrame()
       -> std::variant<std::reference_wrapper<Frame>, ImageAcquireResult> {
      const auto& frame = frames[currentFrame];

      if (const auto res =
              device->getVkDevice().waitForFences(*frame->getInFlightFence(), vk::True, UINT64_MAX);
          res != vk::Result::eSuccess) {
         throw std::runtime_error("Error Waiting for in flight fence");
      }

      const auto result = swapchain->acquireNextImage(frame->getImageAvailableSemaphore());
      if (std::holds_alternative<uint32_t>(result)) {
         frame->setSwapchainImageIndex(std::get<uint32_t>(result));
         currentFrame = (currentFrame + 1) % frames.size();
         device->getVkDevice().resetFences(*frame->getInFlightFence());
         return *frame;
      }

      const auto iar = std::get<ImageAcquireResult>(result);

      if (iar == ImageAcquireResult::NeedsResize) {
         swapchain->recreate();
         commandBufferManager->swapchainRecreated();
      }

      return iar;
   }
}
