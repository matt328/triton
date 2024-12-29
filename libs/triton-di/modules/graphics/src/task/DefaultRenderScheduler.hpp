#pragma once

#include "CommandBufferManager.hpp"
#include "IFrameManager.hpp"
#include "gfx/IRenderScheduler.hpp"
#include "IRenderTask.hpp"
#include "task/CubeRenderTask.hpp"
#include "task/RenderTaskFactory.hpp"

namespace tr {

constexpr auto DepthImageName = "DepthImage";

class TaskGraph;

class DefaultRenderScheduler final : public IRenderScheduler {
public:
  explicit DefaultRenderScheduler(std::shared_ptr<IFrameManager> newFrameManager,
                                  std::shared_ptr<CommandBufferManager> newCommandBufferManager,
                                  std::shared_ptr<queue::Graphics> newGraphicsQueue,
                                  std::shared_ptr<VkResourceManager> newResourceManager,
                                  std::shared_ptr<Swapchain> newSwapchain,
                                  std::shared_ptr<RenderTaskFactory> newRenderTaskFactory,
                                  std::shared_ptr<TaskGraph> newTaskGraph,
                                  const RenderContextConfig& rendererConfig);
  ~DefaultRenderScheduler() override;

  DefaultRenderScheduler(const DefaultRenderScheduler&) = delete;
  DefaultRenderScheduler(DefaultRenderScheduler&&) = delete;
  auto operator=(const DefaultRenderScheduler&) -> DefaultRenderScheduler& = delete;
  auto operator=(DefaultRenderScheduler&&) -> DefaultRenderScheduler& = delete;

  auto executeStaticTasks(Frame& frame) const -> void override;
  auto addStaticTask(std::shared_ptr<IRenderTask> task) -> void override;
  auto recordRenderTasks(Frame& frame) const -> void override;
  auto setupCommandBuffersForFrame(Frame& frame) -> void override;
  auto endFrame(Frame& frame) const -> void override;

private:
  std::shared_ptr<IFrameManager> frameManager;
  std::shared_ptr<CommandBufferManager> commandBufferManager;
  std::shared_ptr<queue::Graphics> graphicsQueue;
  std::shared_ptr<VkResourceManager> resourceManager;
  std::shared_ptr<Swapchain> swapchain;
  std::shared_ptr<RenderTaskFactory> renderTaskFactory;
  std::shared_ptr<TaskGraph> taskGraph;

  std::shared_ptr<CubeRenderTask> cubeRenderTask;

  std::vector<std::shared_ptr<IRenderTask>> staticRenderTasks;

  static auto transitionImage(const vk::raii::CommandBuffer& cmd,
                              const vk::Image& image,
                              vk::ImageLayout currentLayout,
                              vk::ImageLayout newLayout) -> void;

  static auto copyImageToImage(const vk::raii::CommandBuffer& cmd,
                               vk::Image source,
                               vk::Image destination,
                               vk::Extent2D srcSize,
                               vk::Extent2D dstSize) -> void;
};

}
