#pragma once

#include "IFrameManager.hpp"
#include "gfx/RenderContextConfig.hpp"
#include "tr/IEventBus.hpp"

namespace tr {

class CommandBufferManager;
class Swapchain;

class DefaultFrameManager final : public IFrameManager {
public:
  explicit DefaultFrameManager(const RenderContextConfig& newRenderContextConfig,
                               std::shared_ptr<CommandBufferManager> newCommandBufferManager,
                               std::shared_ptr<Device> newDevice,
                               std::shared_ptr<Swapchain> newSwapchain,
                               std::shared_ptr<VkResourceManager> newResourceManager,
                               std::shared_ptr<IEventBus> newEventBus,
                               const std::shared_ptr<IDebugManager>& debugManager);
  ~DefaultFrameManager() override;

  DefaultFrameManager(const DefaultFrameManager&) = delete;
  DefaultFrameManager(DefaultFrameManager&&) = delete;
  auto operator=(const DefaultFrameManager&) -> DefaultFrameManager& = delete;
  auto operator=(DefaultFrameManager&&) -> DefaultFrameManager& = delete;

  auto acquireFrame() -> std::variant<std::reference_wrapper<Frame>, ImageAcquireResult> override;

  [[nodiscard]] auto getFrames() const -> const std::vector<std::unique_ptr<Frame>>& override;

private:
  RenderContextConfig renderConfig;
  std::shared_ptr<CommandBufferManager> commandBufferManager;
  std::shared_ptr<Device> device;
  std::shared_ptr<Swapchain> swapchain;
  std::shared_ptr<VkResourceManager> resourceManager;
  std::shared_ptr<IEventBus> eventBus;

  size_t currentFrame;
  std::vector<std::unique_ptr<Frame>> frames;

  auto handleSwapchainResized(const SwapchainResized& event) -> void;
};

}
