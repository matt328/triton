#include "R3Renderer.hpp"
#include "api/fx/IEventBus.hpp"
#include "api/gw/RenderableResources.hpp"
#include "gfx/IFrameGraph.hpp"
#include "gfx/IFrameManager.hpp"
#include "gfx/QueueTypes.hpp"
#include "mem/ArenaBuffer.hpp"
#include "mem/GeometryBuffer.hpp"
#include "r3/render-pass/RenderPassFactory.hpp"
#include "task/Frame.hpp"
#include "render-pass/GraphicsPassCreateInfo.hpp"
#include "gfx/PassGraphInfo.hpp"

namespace tr {

R3Renderer::R3Renderer(std::shared_ptr<IFrameManager> newFrameManager,
                       std::shared_ptr<queue::Graphics> newGraphicsQueue,
                       std::shared_ptr<IEventBus> newEventBus,
                       std::shared_ptr<Swapchain> newSwapchain,
                       std::shared_ptr<IFrameGraph> newFrameGraph)
    : frameManager{std::move(newFrameManager)},
      graphicsQueue{std::move(newGraphicsQueue)},
      eventBus{std::move(newEventBus)},
      swapchain{std::move(newSwapchain)},
      frameGraph{std::move(newFrameGraph)} {

  renderPassFactory = std::make_shared<RenderPassFactory>();

  const auto forwardPassCreateInfo = GraphicsPassCreateInfo{};
  const auto forwardPassGraphInfo = PassGraphInfo{};

  const auto forwardPass = renderPassFactory->createGraphicsPass(forwardPassCreateInfo);
  frameGraph->addPass(std::move(forwardPass), forwardPassGraphInfo);
}

auto R3Renderer::registerRenderable([[maybe_unused]] const RenderableData& data)
    -> RenderableResources {
  // This won't actually need to be here. The Renderer will already 'expect' the things that can be
  // rendered
  return RenderableResources{};
}

void R3Renderer::update() {
  Log.trace("R3Renderer::update");
}

void R3Renderer::renderNextFrame() {
  const auto result = frameManager->acquireFrame();

  if (std::holds_alternative<ImageAcquireResult>(result)) {
    if (const auto acquireResult = std::get<ImageAcquireResult>(result);
        acquireResult == ImageAcquireResult::Error) {
      Log.warn("Failed to acquire swapchain image");
      return;
    }
  }
  if (std::holds_alternative<Frame*>(result)) {
    const auto* frame = std::get<Frame*>(result);

    frameGraph->execute(frame);
    /*
      TODO(matt): Create the beginnings of a framegraph and call it here.
      Frame graph should initially support 2 render passes, one forward (that just clears the
      screen) and one composition, which copies the resulting image to the swapchain image, using an
      actual renderpass pipeline. basically hardcoded to render a fullscreen quad and use a fragment
      shader to sample the image, and draw it onto the quad.
    */

    endFrame(frame);
  }
}

auto R3Renderer::endFrame(const Frame* frame) -> void {
  buffers.clear();

  constexpr auto waitStages =
      std::array<vk::PipelineStageFlags, 1>{vk::PipelineStageFlagBits::eColorAttachmentOutput};

  const auto submitInfo = vk::SubmitInfo{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &*frame->getImageAvailableSemaphore(),
      .pWaitDstStageMask = waitStages.data(),
      .commandBufferCount = static_cast<uint32_t>(buffers.size()),
      .pCommandBuffers = buffers.data(),
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &*frame->getRenderFinishedSemaphore(),
  };

  std::string msg = fmt::format("Submitting Queue for frame {}", frame->getIndex());
  TracyMessage(msg.data(), msg.size());

  try {
    const auto fence = *frame->getInFlightFence();
    graphicsQueue->getQueue().submit(submitInfo, fence);
    eventBus->emit(FrameEndEvent{fence});
  } catch (const std::exception& ex) {
    Log.error("Failed to submit command buffer submission {}", ex.what());
  }

  try {
    const auto swapchainImageIndex = frame->getSwapchainImageIndex();
    const auto chain = swapchain->getSwapchain();

    const auto presentInfo =
        vk::PresentInfoKHR{.waitSemaphoreCount = 1,
                           .pWaitSemaphores = &*frame->getRenderFinishedSemaphore(),
                           .swapchainCount = 1,
                           .pSwapchains = &chain,
                           .pImageIndices = &swapchainImageIndex};

    std::string msg = fmt::format("Presenting frame {}", frame->getIndex());
    TracyMessage(msg.data(), msg.size());
    if (const auto result2 = graphicsQueue->getQueue().presentKHR(presentInfo);
        result2 == vk::Result::eSuboptimalKHR || result2 == vk::Result::eErrorOutOfDateKHR) {
      Log.trace("Swapchain Needs Resized");
    }
  } catch (const std::exception& ex) { Log.trace("Swapchain needs recreated: {0}", ex.what()); }
}

void R3Renderer::waitIdle() {
}

void R3Renderer::setRenderData([[maybe_unused]] const RenderData& renderData) {
}

auto R3Renderer::getGeometryBuffer() -> GeometryBuffer& {
  return *geometryBuffer;
}

}
