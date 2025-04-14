#pragma once

#include "bk/Rando.hpp"
#include "gfx/IFrameManager.hpp"
#include "gfx/RenderContextConfig.hpp"
#include "api/fx/IEventBus.hpp"

namespace tr {

class CommandBufferManager;
class Swapchain;
class VkResourceManager;
class BufferRegistry;

class DefaultFrameManager final : public IFrameManager {
public:
  explicit DefaultFrameManager(const RenderContextConfig& newRenderContextConfig,
                               std::shared_ptr<CommandBufferManager> newCommandBufferManager,
                               std::shared_ptr<Device> newDevice,
                               std::shared_ptr<Swapchain> newSwapchain,
                               std::shared_ptr<VkResourceManager> newResourceManager,
                               std::shared_ptr<IEventBus> newEventBus,
                               std::shared_ptr<IDebugManager> debugManager,
                               std::shared_ptr<BufferRegistry> newBufferRegistry);
  ~DefaultFrameManager() override;

  DefaultFrameManager(const DefaultFrameManager&) = delete;
  DefaultFrameManager(DefaultFrameManager&&) = delete;
  auto operator=(const DefaultFrameManager&) -> DefaultFrameManager& = delete;
  auto operator=(DefaultFrameManager&&) -> DefaultFrameManager& = delete;

  auto acquireFrame() -> std::variant<Frame*, ImageAcquireResult> override;

  [[nodiscard]] auto getFrames() const -> const std::vector<std::unique_ptr<Frame>>& override;

  // For each frame, create a drawImage and store its handle associated with the same
  // LogicalImageHandle
  auto registerPerFrameDrawImage(vk::Extent2D extent) -> LogicalImageHandle override;
  auto registerPerFrameDepthImage(vk::Extent2D extent, vk::Format format)
      -> LogicalImageHandle override;
  auto createPerFrameBuffer(const BufferUsageProfile& profile, size_t drawContextId)
      -> LogicalBufferHandle override;

private:
  RenderContextConfig renderConfig;
  std::shared_ptr<CommandBufferManager> commandBufferManager;
  std::shared_ptr<Device> device;
  std::shared_ptr<Swapchain> swapchain;
  std::shared_ptr<VkResourceManager> resourceManager;
  std::shared_ptr<IEventBus> eventBus;
  std::shared_ptr<BufferRegistry> bufferRegistry;

  size_t currentFrame;
  std::vector<std::unique_ptr<Frame>> frames;

  MapKey imageKeygen;
  MapKey bufferKeygen;

  auto handleSwapchainResized(const SwapchainResized& event) -> void;
};

}
