#pragma once

#include "TerrainTask.hpp"
#include "api/gfx/ImGuiSystem.hpp"
#include "api/gw/RenderData.hpp"
#include "gfx/RenderContextConfig.hpp"
#include "api/fx/Events.hpp"
#include "IGuiSystem.hpp"
#include "vk/CommandBufferManager.hpp"
#include "gfx/IFrameManager.hpp"
#include "gfx/IRenderScheduler.hpp"
#include "IRenderTask.hpp"
#include "task/ComputeTask.hpp"
#include "task/IndirectRenderTask.hpp"
#include "task/RenderTaskFactory.hpp"

namespace tr {

class DefaultRenderScheduler final : public IRenderScheduler {
public:
  explicit DefaultRenderScheduler(std::shared_ptr<IFrameManager> newFrameManager,
                                  std::shared_ptr<CommandBufferManager> newCommandBufferManager,
                                  std::shared_ptr<queue::Graphics> newGraphicsQueue,
                                  std::shared_ptr<VkResourceManager> newResourceManager,
                                  std::shared_ptr<Swapchain> newSwapchain,
                                  std::shared_ptr<RenderTaskFactory> newRenderTaskFactory,
                                  std::shared_ptr<IGuiSystem> newGuiSystem,
                                  std::shared_ptr<BufferManager> newBufferManager,
                                  const RenderContextConfig& rendererConfig,
                                  std::shared_ptr<IEventBus> newEventBus);
  ~DefaultRenderScheduler() override;

  DefaultRenderScheduler(const DefaultRenderScheduler&) = delete;
  DefaultRenderScheduler(DefaultRenderScheduler&&) = delete;
  auto operator=(const DefaultRenderScheduler&) -> DefaultRenderScheduler& = delete;
  auto operator=(DefaultRenderScheduler&&) -> DefaultRenderScheduler& = delete;

  auto updatePerFrameRenderData(Frame* frame, const RenderData& renderData) -> void override;

  auto executeTasks(Frame* frame, bool recordTasks) const -> void override;
  auto recordRenderTasks(Frame* frame, bool recordTasks) -> void override;
  auto endFrame(Frame* frame) -> void override;

private:
  std::shared_ptr<IFrameManager> frameManager;
  std::shared_ptr<CommandBufferManager> commandBufferManager;
  std::shared_ptr<queue::Graphics> graphicsQueue;
  std::shared_ptr<VkResourceManager> resourceManager;
  std::shared_ptr<Swapchain> swapchain;
  std::shared_ptr<RenderTaskFactory> renderTaskFactory;
  std::shared_ptr<IGuiSystem> guiSystem;
  std::shared_ptr<BufferManager> bufferManager;
  std::shared_ptr<IEventBus> eventBus;

  std::shared_ptr<IndirectRenderTask> indirectRenderTask;
  std::shared_ptr<ComputeTask> computeTask;
  std::shared_ptr<StaticTask> staticRenderTask;
  std::shared_ptr<TerrainTask> terrainTask;

  std::vector<std::shared_ptr<IRenderTask>> staticRenderTasks;

  vk::Viewport viewport;
  vk::Rect2D snezzor;

  bool tasksRecorded = false;

  std::vector<vk::CommandBuffer> buffers;

  RenderContextConfig renderConfig;

  auto handleSwapchainResized(const SwapchainResized& event) -> void;

  auto createStaticBuffers(const std::unique_ptr<Frame>& frame) -> void;
  auto createDynamicBuffers(const std::unique_ptr<Frame>& frame) -> void;
  auto createTerrainBuffers(const std::unique_ptr<Frame>& frame) -> void;

  auto updateStaticBuffers(Frame* frame, const RenderData& renderData) -> void;
  auto updateDynamicBuffers(Frame* frame, const RenderData& renderData) -> void;
  auto updateTerrainBuffers(Frame* frame, const RenderData& renderData) -> void;

  static auto transitionImage(const vk::raii::CommandBuffer& cmd,
                              const vk::Image& image,
                              vk::ImageLayout currentLayout,
                              vk::ImageLayout newLayout) -> void;

  static auto copyImageToImage(const vk::raii::CommandBuffer& cmd,
                               vk::Image source,
                               vk::Image destination,
                               vk::Extent2D srcSize,
                               vk::Extent2D dstSize) -> void;

  static auto insertBarrier(const vk::raii::CommandBuffer& cmd, const Buffer& buffer) -> void;
};

}
