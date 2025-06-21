#pragma once

#include "api/fx/IStateBuffer.hpp"
#include "api/gfx/RenderStyle.hpp"
#include "buffers/ManagedBuffer.hpp"
#include "gfx/IRenderContext.hpp"
#include "gfx/RenderContextConfig.hpp"
#include "gfx/GeometryHandleMapper.hpp"
#include "img/ManagedImage.hpp"

namespace tr {

class IFrameManager;
class Swapchain;
class IFrameGraph;
class RenderPassFactory;
struct FrameGraphResult;
class CommandBufferManager;
class BufferSystem;
class GeometryBufferPack;
class FrameState;
class GeometryAllocator;
class ContextFactory;
class ImageManager;
class Frame;
class ResourceAliasRegistry;
class IRenderPass;
class IShaderBinding;
class IShaderBindingFactory;
class DSLayoutManager;
class DSLayout;

namespace queue {
class Graphics;
}

const std::filesystem::path SHADER_ROOT = std::filesystem::current_path() / "assets" / "shaders";

struct GlobalBuffers {
  LogicalHandle<ManagedBuffer> drawCommands;
  LogicalHandle<ManagedBuffer> drawCounts;
  LogicalHandle<ManagedBuffer> drawMetadata;
  LogicalHandle<ManagedBuffer> objectData;
  LogicalHandle<ManagedBuffer> objectPositions;
  LogicalHandle<ManagedBuffer> objectRotations;
  LogicalHandle<ManagedBuffer> objectScales;
  LogicalHandle<ManagedBuffer> geometryRegion;
  LogicalHandle<ManagedBuffer> frameData;
  LogicalHandle<ManagedBuffer> resourceTable;
};

struct GlobalImages {
  LogicalHandle<ManagedImage> forwardColorImage;
  LogicalHandle<ManagedImage> forwardDepthImage;
  LogicalHandle<ManagedImage> imguiColorImage;
};

struct GlobalShaderBindings {
  Handle<DSLayout> defaultBindingLayout;
  LogicalHandle<IShaderBinding> defaultBinding;
  Handle<vk::raii::Sampler> defaultSampler;
};

class R3Renderer : public tr::IRenderContext {
public:
  R3Renderer(RenderContextConfig newRenderConfig,
             std::shared_ptr<IFrameManager> newFrameManager,
             std::shared_ptr<queue::Graphics> newGraphicsQueue,
             std::shared_ptr<Swapchain> newSwapchain,
             std::shared_ptr<IFrameGraph> newFrameGraph,
             std::shared_ptr<RenderPassFactory> newRenderPassFactory,
             std::shared_ptr<CommandBufferManager> newCommandBufferManager,
             std::shared_ptr<BufferSystem> newBufferSystem,
             std::shared_ptr<ContextFactory> newDrawContextFactory,
             std::shared_ptr<IStateBuffer> newStateBuffer,
             std::shared_ptr<ImageManager> newImageManager,
             std::shared_ptr<GeometryBufferPack> newGeometryBufferPack,
             std::shared_ptr<FrameState> newFrameState,
             std::shared_ptr<GeometryAllocator> newGeometryAllocator,
             std::shared_ptr<GeometryHandleMapper> newGeometryHandleMapper,
             std::shared_ptr<ResourceAliasRegistry> newAliasRegistry,
             std::shared_ptr<IShaderBindingFactory> newShaderBindingFactory,
             std::shared_ptr<DSLayoutManager> newLayoutManager);
  ~R3Renderer() override = default;

  R3Renderer(const R3Renderer&) = delete;
  R3Renderer(R3Renderer&&) = delete;
  auto operator=(const R3Renderer&) -> R3Renderer& = delete;
  auto operator=(R3Renderer&&) -> R3Renderer& = delete;

  void renderNextFrame() override;
  void waitIdle() override;

private:
  RenderContextConfig rendererConfig;
  std::shared_ptr<IFrameManager> frameManager;
  std::shared_ptr<queue::Graphics> graphicsQueue;
  std::shared_ptr<Swapchain> swapchain;
  std::shared_ptr<IFrameGraph> frameGraph;
  std::shared_ptr<RenderPassFactory> renderPassFactory;
  std::shared_ptr<CommandBufferManager> commandBufferManager;
  std::shared_ptr<BufferSystem> bufferSystem;
  std::shared_ptr<ContextFactory> drawContextFactory;
  std::shared_ptr<IStateBuffer> stateBuffer;
  std::shared_ptr<ImageManager> imageManager;
  std::shared_ptr<GeometryBufferPack> geometryBufferPack;
  std::shared_ptr<FrameState> frameState;
  std::shared_ptr<GeometryAllocator> geometryAllocator;
  std::shared_ptr<GeometryHandleMapper> geometryHandleMapper;
  std::shared_ptr<ResourceAliasRegistry> aliasRegistry;
  std::shared_ptr<IShaderBindingFactory> shaderBindingFactory;
  std::shared_ptr<DSLayoutManager> layoutManager;

  std::vector<vk::CommandBuffer> buffers;

  GlobalBuffers globalBuffers{};
  GlobalImages globalImages{};
  GlobalShaderBindings globalShaderBindings{};

  std::vector<GpuGeometryRegionData> geometryRegionContents;

  auto createGlobalBuffers() -> void;
  auto createGlobalImages() -> void;
  auto createGlobalShaderBindings() -> void;
  auto createComputeCullingPass() -> std::unique_ptr<IRenderPass>;
  auto createForwardRenderPass() -> std::unique_ptr<IRenderPass>;
  auto createCompositionRenderPass() -> std::unique_ptr<IRenderPass>;
  auto createImGuiPass() -> std::unique_ptr<IRenderPass>;
  auto createPresentPass() -> std::unique_ptr<IRenderPass>;
  auto endFrame(const Frame* frame, const FrameGraphResult& result) -> void;

  auto buildFrameState(std::vector<GpuObjectData>& objectData,
                       std::vector<StateHandles>& stateHandles,
                       std::vector<GpuGeometryRegionData>& regionBuffer) -> void;
};
}
