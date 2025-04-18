#pragma once

#include "dd/render-pass/RenderPass.hpp"
#include "gfx/IRenderContext.hpp"
#include "gfx/RenderContextConfig.hpp"
#include "dd/Renderable.hpp"

namespace tr {

class RenderConfigRegistry;
class DrawContextFactory;
class IFrameManager;
class RenderPassFactory;
class Swapchain;
class CommandBufferManager;
class IEventBus;
class IShaderModuleFactory;

namespace queue {
class Graphics;
}

class DDRenderer : public IRenderContext {
public:
  DDRenderer(RenderContextConfig newConfig,
             std::shared_ptr<RenderConfigRegistry> newRenderConfigRegistry,
             std::shared_ptr<DrawContextFactory> newDrawContextFactory,
             std::shared_ptr<IFrameManager> newFrameManager,
             std::shared_ptr<RenderPassFactory> newRenderPassFactory,
             std::shared_ptr<CommandBufferManager> newCommandBufferManager,
             std::shared_ptr<Swapchain> swapchain,
             std::shared_ptr<queue::Graphics> newGraphicsQueue,
             std::shared_ptr<IEventBus> newEventBus,
             std::shared_ptr<IShaderModuleFactory> newShaderModuleFactory);
  ~DDRenderer() override = default;

  DDRenderer(const DDRenderer&) = default;
  DDRenderer(DDRenderer&&) = delete;
  auto operator=(const DDRenderer&) -> DDRenderer& = default;
  auto operator=(DDRenderer&&) -> DDRenderer& = delete;

  auto update() -> void override;
  auto renderNextFrame() -> void override;
  auto waitIdle() -> void override;
  auto setRenderData(const RenderData& newRenderData) -> void override;

  auto registerRenderable(const RenderableData& data) -> RenderableResources override;

private:
  RenderContextConfig rendererConfig;
  std::shared_ptr<RenderConfigRegistry> renderConfigRegistry;
  std::shared_ptr<DrawContextFactory> drawContextFactory;
  std::shared_ptr<IFrameManager> frameManager;
  std::shared_ptr<RenderPassFactory> renderPassFactory;
  std::shared_ptr<CommandBufferManager> commandBufferManager;
  std::shared_ptr<Swapchain> swapchain;
  std::shared_ptr<queue::Graphics> graphicsQueue;
  std::shared_ptr<IEventBus> eventBus;
  std::shared_ptr<IShaderModuleFactory> shaderModuleFactory;

  std::vector<std::unique_ptr<RenderPass>> renderPasses;

  RenderData renderData;
  std::vector<Renderable> renderables;
  std::unordered_map<RenderConfigHandle, std::vector<Renderable>> categorizedRenderables;
  std::vector<vk::CommandBuffer> buffers;

  auto preRender(const Frame* frame) -> void;
  auto combineImages(const Frame* frame) -> void;
  auto endFrame(const Frame* frame) -> void;

  auto createForwardRenderPass() -> void;
  auto createUIRenderPass() -> void;
};

}
