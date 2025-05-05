#pragma once

#include "api/gfx/RenderStyle.hpp"
#include "buffers/ManagedBuffer.hpp"
#include "gfx/IRenderContext.hpp"
#include "gfx/RenderContextConfig.hpp"
#include "r3/render-pass/GraphicsPass.hpp"

namespace tr {

class IFrameManager;
class IEventBus;
class Swapchain;
class IFrameGraph;
class RenderPassFactory;
struct FrameGraphResult;
class CommandBufferManager;
class BufferSystem;

namespace queue {
class Graphics;
}

const std::filesystem::path SHADER_ROOT = std::filesystem::current_path() / "assets" / "shaders";

struct GlobalBuffers {
  LogicalHandle<ManagedBuffer> drawCommands;
  LogicalHandle<ManagedBuffer> drawCounts;
  LogicalHandle<ManagedBuffer> drawMetadata;
  LogicalHandle<ManagedBuffer> objectData;
  Handle<ManagedBuffer> geometryEntry;
  Handle<ManagedBuffer> geometryPositions;
  Handle<ManagedBuffer> geometryColors;
};

class R3Renderer : public tr::IRenderContext {
public:
  R3Renderer(RenderContextConfig newRenderConfig,
             std::shared_ptr<IFrameManager> newFrameManager,
             std::shared_ptr<queue::Graphics> newGraphicsQueue,
             std::shared_ptr<IEventBus> newEventBus,
             std::shared_ptr<Swapchain> newSwapchain,
             std::shared_ptr<IFrameGraph> newFrameGraph,
             std::shared_ptr<RenderPassFactory> newRenderPassFactory,
             std::shared_ptr<CommandBufferManager> newCommandBufferManager,
             std::shared_ptr<BufferSystem> newBufferSystem);
  ~R3Renderer() override = default;

  R3Renderer(const R3Renderer&) = delete;
  R3Renderer(R3Renderer&&) = delete;
  auto operator=(const R3Renderer&) -> R3Renderer& = delete;
  auto operator=(R3Renderer&&) -> R3Renderer& = delete;

  auto registerRenderable(const RenderableData& data) -> RenderableResources override;

  void update() override;
  void renderNextFrame() override;
  void waitIdle() override;

  void setRenderData(const RenderData& renderData) override;

private:
  RenderContextConfig rendererConfig;
  std::shared_ptr<IFrameManager> frameManager;
  std::shared_ptr<queue::Graphics> graphicsQueue;
  std::shared_ptr<IEventBus> eventBus;
  std::shared_ptr<Swapchain> swapchain;
  std::shared_ptr<IFrameGraph> frameGraph;
  std::shared_ptr<RenderPassFactory> renderPassFactory;
  std::shared_ptr<CommandBufferManager> commandBufferManager;
  std::shared_ptr<BufferSystem> bufferSystem;

  std::unordered_map<RenderPassType, GraphicsPass> renderPasses;

  std::vector<vk::CommandBuffer> buffers;

  GlobalBuffers globalBuffers{};

  auto createGlobalBuffers() -> void;
  auto createComputeCullingPass() -> void;
  auto createForwardRenderPass() -> void;
  auto createCompositionRenderPass() -> void;
  auto endFrame(const Frame* frame, const FrameGraphResult& result) -> void;
};
}
