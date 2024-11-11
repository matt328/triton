#include "DefaultRenderContext.hpp"
#include "DepthResources.hpp"
#include "geo/VertexAttributes.hpp"
#include "pipeline/StaticModelPipeline.hpp"
#include "renderer/RendererFactory.hpp"
#include "sb/ILayoutFactory.hpp"
#include "sb/IShaderBindingFactory.hpp"
#include "gfx/IGraphicsDevice.hpp"
#include "FrameManager.hpp"

namespace tr::gfx {

   DefaultRenderContext::DefaultRenderContext(
       std::shared_ptr<IGraphicsDevice> graphicsDevice,
       std::shared_ptr<sb::ILayoutFactory> newLayoutFactory,
       std::shared_ptr<sb::IShaderBindingFactory> newShaderBindingFactory,
       std::shared_ptr<pipe::IShaderCompiler> newShaderCompiler,
       std::shared_ptr<rd::RendererFactory> newRendererFactory)
       : graphicsDevice{std::move(graphicsDevice)},
         layoutFactory{std::move(newLayoutFactory)},
         shaderBindingFactory{std::move(newShaderBindingFactory)},
         shaderCompiler{std::move(newShaderCompiler)},
         rendererFactory{std::move(newRendererFactory)} {
      Log.trace("Constructing DefaultRenderContext");

      const auto defaultSetLayouts = std::vector<vk::DescriptorSetLayout>{};
      const auto defaultVertexComponents = std::vector<geo::VertexComponent>{};

      defaultRenderer = rendererFactory->createRenderer(
          rd::RendererConfig{.rendererType = rd::RendererType::StaticModel,
                             .setLayouts = defaultSetLayouts,
                             .vertexComponents = defaultVertexComponents});

      // TODO(matt): other renderers

      depthResources = std::make_shared<DepthResources>(graphicsDevice);

      frameManager = std::make_shared<FrameManager>(2,
                                                    graphicsDevice,
                                                    depthResources->getImageView(),
                                                    newShaderBindingFactory);
   }

   void DefaultRenderContext::render() {
   }

   void DefaultRenderContext::waitIdle() {
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