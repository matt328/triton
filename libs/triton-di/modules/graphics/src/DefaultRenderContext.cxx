#include "DefaultRenderContext.hpp"
#include "DepthResources.hpp"
#include "geo/VertexAttributes.hpp"
#include "tr/Events.hpp"
#include "tr/IEventBus.hpp"
#include "tr/IGameplaySystem.hpp"
#include "pipeline/StaticModelPipeline.hpp"
#include "renderer/RendererFactory.hpp"
#include "sb/ILayoutFactory.hpp"
#include "sb/IShaderBindingFactory.hpp"
#include "gfx/IGraphicsDevice.hpp"
#include "FrameManager.hpp"
#include "Frame.hpp"
#include "renderer/IRenderer.hpp"

namespace tr {

DefaultRenderContext::DefaultRenderContext(
    std::shared_ptr<IGraphicsDevice> newGraphicsDevice,
    std::shared_ptr<ILayoutFactory> newLayoutFactory,
    std::shared_ptr<IShaderBindingFactory> newShaderBindingFactory,
    std::shared_ptr<IShaderCompiler> newShaderCompiler,
    std::shared_ptr<rd::RendererFactory> newRendererFactory,
    std::shared_ptr<IGameplaySystem> newGameplaySystem,
    std::shared_ptr<IGuiSystem> newGuiSystem,
    std::shared_ptr<IEventBus> newEventBus)
    : graphicsDevice{std::move(newGraphicsDevice)},
      layoutFactory{std::move(newLayoutFactory)},
      shaderBindingFactory{std::move(newShaderBindingFactory)},
      shaderCompiler{std::move(newShaderCompiler)},
      rendererFactory{std::move(newRendererFactory)},
      gameplaySystem{std::move(newGameplaySystem)},
      guiSystem{std::move(newGuiSystem)},
      eventBus{std::move(newEventBus)} {

   Log.trace("Constructing DefaultRenderContext");

   const auto& extent = graphicsDevice->getSwapchainExtent();

   eventBus->emit(SwapchainResized{extent.width, extent.height});

   // TODO(matt) These shouldn't be empty
   auto defaultSetLayouts = std::vector<vk::DescriptorSetLayout>{};
   defaultSetLayouts.push_back(layoutFactory->getVkLayout(LayoutHandle::Bindless));
   defaultSetLayouts.push_back(layoutFactory->getVkLayout(LayoutHandle::ObjectData));
   defaultSetLayouts.push_back(layoutFactory->getVkLayout(LayoutHandle::PerFrame));

   auto vec = std::vector{VertexComponent::Position,
                          VertexComponent::Color,
                          VertexComponent::UV,
                          VertexComponent::Normal};

   defaultRenderer = rendererFactory->createRenderer(
       rd::RendererConfig{.rendererType = rd::RendererType::StaticModel,
                          .setLayouts = defaultSetLayouts,
                          .vertexComponents = vec});

   depthResources = std::make_shared<DepthResources>(graphicsDevice);

   frameManager = std::make_shared<FrameManager>(2,
                                                 graphicsDevice,
                                                 depthResources->getImageView(),
                                                 shaderBindingFactory);
}

DefaultRenderContext::~DefaultRenderContext() {
   Log.trace("Destroying DefaultRenderContext");
   DefaultRenderContext::waitIdle();
}

void DefaultRenderContext::render() {
   auto& currentFrame = frameManager->getCurrentFrame();

   currentFrame.awaitInFlightFence();

   const auto status = currentFrame.acquireSwapchainImage();

   if (status == AcquireResult::NeedsResize) {
      graphicsDevice->waitIdle();
      frameManager->destroySwapchainResources();
      graphicsDevice->recreateSwapchain();
      frameManager->createSwapchainResources();
      return;
   }
   if (status == AcquireResult::Error) {
      Log.error("Error acquiring next image");
      return;
   }

   currentFrame.resetInFlightFence();

   {
      auto lock = std::lock_guard{renderDataMutex};
      LockableName(renderDataMutex, "SetRenderData", 13);
      LockMark(renderDataMutex);
      if (!renderData.skinnedMeshData.empty()) {
         Log.trace("Skinned Mesh Data Present");
      }
      currentFrame.applyRenderData(renderData);
   }

   {
      const auto imageInfoList = graphicsDevice->getTextures();
      currentFrame.applyTextures(imageInfoList.get());
   }

   currentFrame.render(defaultRenderer, getViewportAndScissor());

   currentFrame.end3d();

   currentFrame.renderGuiSystem(guiSystem);

   if (currentFrame.present()) {
      graphicsDevice->waitIdle();
      frameManager->destroySwapchainResources();
      graphicsDevice->recreateSwapchain();
      frameManager->createSwapchainResources();
      eventBus->emit(SwapchainResized{.width = graphicsDevice->getSwapchainExtent().width,
                                      .height = graphicsDevice->getSwapchainExtent().height});
      return;
   }

   frameManager->nextFrame();
}

void DefaultRenderContext::waitIdle() {
   graphicsDevice->waitIdle();
}

void DefaultRenderContext::setRenderData(const RenderData& newRenderData) {
   auto lock = std::lock_guard{renderDataMutex};
   LockableName(renderDataMutex, "SetRenderData", 13);
   LockMark(renderDataMutex);
   renderData = newRenderData;
}

[[nodiscard]] auto DefaultRenderContext::getViewportAndScissor()
    -> std::tuple<vk::Viewport, vk::Rect2D> {
   const auto viewportSize = graphicsDevice->getSwapchainExtent();
   const auto mainViewport = vk::Viewport{.x = 0.f,
                                          .y = 0.f,
                                          .width = static_cast<float>(viewportSize.width),
                                          .height = static_cast<float>(viewportSize.height),
                                          .minDepth = 0.f,
                                          .maxDepth = 1.f};

   const auto mainScissor = vk::Rect2D{.offset = {0, 0}, .extent = viewportSize};

   return {mainViewport, mainScissor};
}
}

