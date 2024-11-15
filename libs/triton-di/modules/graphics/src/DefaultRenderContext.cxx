#include "DefaultRenderContext.hpp"
#include "DepthResources.hpp"
#include "geo/VertexAttributes.hpp"
#include "gp/IGameplaySystem.hpp"
#include "pipeline/StaticModelPipeline.hpp"
#include "renderer/RendererFactory.hpp"
#include "sb/ILayoutFactory.hpp"
#include "sb/IShaderBindingFactory.hpp"
#include "gfx/IGraphicsDevice.hpp"
#include "FrameManager.hpp"
#include "Frame.hpp"
#include "renderer/IRenderer.hpp"
#include <mutex>

namespace tr::gfx {

   DefaultRenderContext::DefaultRenderContext(
       std::shared_ptr<IGraphicsDevice> newGraphicsDevice,
       std::shared_ptr<sb::ILayoutFactory> newLayoutFactory,
       std::shared_ptr<sb::IShaderBindingFactory> newShaderBindingFactory,
       std::shared_ptr<pipe::IShaderCompiler> newShaderCompiler,
       std::shared_ptr<rd::RendererFactory> newRendererFactory,
       std::shared_ptr<gp::IGameplaySystem> newGameplaySystem)
       : graphicsDevice{std::move(newGraphicsDevice)},
         layoutFactory{std::move(newLayoutFactory)},
         shaderBindingFactory{std::move(newShaderBindingFactory)},
         shaderCompiler{std::move(newShaderCompiler)},
         rendererFactory{std::move(newRendererFactory)},
         gameplaySystem{std::move(newGameplaySystem)} {
      Log.trace("Constructing DefaultRenderContext");

      // TODO(matt) These shouldn't be empty
      const auto defaultSetLayouts = std::vector<vk::DescriptorSetLayout>{};
      const auto defaultVertexComponents = std::vector<geo::VertexComponent>{};

      defaultRenderer = rendererFactory->createRenderer(
          rd::RendererConfig{.rendererType = rd::RendererType::StaticModel,
                             .setLayouts = defaultSetLayouts,
                             .vertexComponents = defaultVertexComponents});

      depthResources = std::make_shared<DepthResources>(graphicsDevice);

      frameManager = std::make_shared<FrameManager>(2,
                                                    graphicsDevice,
                                                    depthResources->getImageView(),
                                                    shaderBindingFactory);
   }

   void DefaultRenderContext::render() {
      auto& currentFrame = frameManager->getCurrentFrame();

      currentFrame.awaitInFlightFence();

      const auto status = currentFrame.acquireSwapchainImage();

      if (status == AcquireResult::NeedsResize) {
         resizeSwapchain();
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
         currentFrame.applyRenderData(renderData);
      }

      {
         const auto imageInfoList = graphicsDevice->getTextures();
         currentFrame.applyTextures(imageInfoList.get());
      }

      currentFrame.render(defaultRenderer, getViewportAndScissor());

      if (currentFrame.present()) {
         resizeSwapchain();
      }

      frameManager->nextFrame();
   }

   void DefaultRenderContext::resizeSwapchain() {
   }

   void DefaultRenderContext::waitIdle() {
   }

   void DefaultRenderContext::setRenderData(const cm::gpu::RenderData& newRenderData) {
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