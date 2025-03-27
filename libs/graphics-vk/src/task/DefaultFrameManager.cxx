#include "DefaultFrameManager.hpp"
#include "gfx/RenderContextConfig.hpp"
#include "Frame.hpp"
#include "Maths.hpp"
#include "api/fx/Events.hpp"
#include "vk/core/Swapchain.hpp"

namespace tr {

DefaultFrameManager::DefaultFrameManager(
    const RenderContextConfig& newRenderContextConfig,
    std::shared_ptr<CommandBufferManager> newCommandBufferManager,
    std::shared_ptr<Device> newDevice,
    std::shared_ptr<Swapchain> newSwapchain,
    std::shared_ptr<VkResourceManager> newResourceManager,
    std::shared_ptr<IEventBus> newEventBus,
    std::shared_ptr<IDebugManager> debugManager)
    : renderConfig{newRenderContextConfig},
      commandBufferManager{std::move(newCommandBufferManager)},
      device{std::move(newDevice)},
      swapchain{std::move(newSwapchain)},
      resourceManager{std::move(newResourceManager)},
      eventBus{std::move(newEventBus)},
      currentFrame{0} {

  for (uint8_t i = 0; i < renderConfig.framesInFlight; ++i) {

    auto fence = device->getVkDevice().createFence(
        vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});

    auto acquireImageSemaphore = device->getVkDevice().createSemaphore({});
    debugManager->setObjectName(*acquireImageSemaphore,
                                "Semaphore-AcquireImage-Frame_" + std::to_string(i));

    auto renderFinishedSemaphore = device->getVkDevice().createSemaphore({});
    debugManager->setObjectName(*renderFinishedSemaphore,
                                "Semaphore-RenderFinished-Frame_" + std::to_string(i));
    auto computeFinishedSemaphore = device->getVkDevice().createSemaphore({});
    debugManager->setObjectName(*computeFinishedSemaphore,
                                "Semaphore-ComputeFinished-Frame_" + std::to_string(i));

    const auto startCmdBufferHandle = commandBufferManager->createGraphicsCommandBuffer();
    const auto endCmdBufferHandle = commandBufferManager->createGraphicsCommandBuffer();
    const auto mainCmdBufferHandle = commandBufferManager->createGraphicsCommandBuffer();

    frames.push_back(std::make_unique<Frame>(static_cast<uint8_t>(frames.size()),
                                             std::move(fence),
                                             std::move(acquireImageSemaphore),
                                             std::move(renderFinishedSemaphore),
                                             std::move(computeFinishedSemaphore),
                                             startCmdBufferHandle,
                                             endCmdBufferHandle,
                                             mainCmdBufferHandle));
  }

  const auto drawImageExtent = vk::Extent2D{
      .width = maths::scaleNumber(swapchain->getImageExtent().width, renderConfig.renderScale),
      .height = maths::scaleNumber(swapchain->getImageExtent().height, renderConfig.renderScale)};

  for (auto& frame : frames) {
    auto drawImageHandle =
        resourceManager->createDrawImageAndView(frame->getIndexedName("Image-Draw-Frame_"),
                                                drawImageExtent);
    frame->setDrawImageHandle(drawImageHandle);
  }

  eventBus->subscribe<SwapchainResized>(
      [&](SwapchainResized event) { handleSwapchainResized(event); });
}

DefaultFrameManager::~DefaultFrameManager() {
  Log.trace("Destroying DefaultFrameManager");
  device->waitIdle();
  frames.clear();
}

auto DefaultFrameManager::handleSwapchainResized(const SwapchainResized& event) -> void {

  const auto drawImageExtent =
      vk::Extent2D{.width = maths::scaleNumber(event.width, renderConfig.renderScale),
                   .height = maths::scaleNumber(event.height, renderConfig.renderScale)};

  for (auto& frame : frames) {
    resourceManager->destroyDrawImageAndView(frame->getDrawImageHandle());
    auto drawImageHandle =
        resourceManager->createDrawImageAndView(frame->getIndexedName("Image-Draw-Frame_"),
                                                drawImageExtent);
    frame->setDrawImageHandle(drawImageHandle);
    frame->setupRenderingInfo(resourceManager);
  }
}

auto DefaultFrameManager::acquireFrame() -> std::variant<Frame*, ImageAcquireResult> {
  currentFrame = (currentFrame + 1) % frames.size();
  auto* frame = frames[currentFrame].get();

  const std::string msg = fmt::format("Waiting for fence for frame {}", currentFrame);
  TracyMessage(msg.data(), msg.size());

  {
    ZoneNamedN(var, "waitForFences", true);
    const uint64_t timeout = 1'000'000; // 1ms
    vk::Result res;
    do {
      res = device->getVkDevice().waitForFences(*frame->getInFlightFence(), vk::True, timeout);
    } while (res == vk::Result::eTimeout);

    if (res == vk::Result::eSuccess) {
      device->getVkDevice().resetFences(*frame->getInFlightFence());
    }
  }

  std::variant<uint32_t, ImageAcquireResult> result{};

  {
    ZoneNamedN(var, "acquireNextImage", true);
    result = swapchain->acquireNextImage(frame->getImageAvailableSemaphore());
  }

  if (std::holds_alternative<uint32_t>(result)) {
    frame->setSwapchainImageIndex(std::get<uint32_t>(result));
    return frame;
  }

  const auto iar = std::get<ImageAcquireResult>(result);

  if (iar == ImageAcquireResult::NeedsResize) {
    swapchain->recreate();
  }

  return iar;
}

auto DefaultFrameManager::getFrames() const -> const std::vector<std::unique_ptr<Frame>>& {
  return frames;
}

}
