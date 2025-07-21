#pragma once

#include "api/fx/Events.hpp"
#include "bk/HandleGenerator.hpp"
#include "bk/Rando.hpp"
#include "gfx/IFrameManager.hpp"
#include "gfx/RenderContextConfig.hpp"
#include "img/ManagedImage.hpp"

namespace tr {

class CommandBufferManager;
class Swapchain;
class BufferRegistry;
class IEventQueue;
class FrameState;

class DefaultFrameManager final : public IFrameManager {
public:
  explicit DefaultFrameManager(const RenderContextConfig& newRenderContextConfig,
                               std::shared_ptr<CommandBufferManager> newCommandBufferManager,
                               std::shared_ptr<Device> newDevice,
                               std::shared_ptr<Swapchain> newSwapchain,
                               std::shared_ptr<IEventQueue> newEventQueue,
                               std::shared_ptr<FrameState> newFrameState,
                               std::shared_ptr<IDebugManager> newDebugManager);
  ~DefaultFrameManager() override;

  DefaultFrameManager(const DefaultFrameManager&) = delete;
  DefaultFrameManager(DefaultFrameManager&&) = delete;
  auto operator=(const DefaultFrameManager&) -> DefaultFrameManager& = delete;
  auto operator=(DefaultFrameManager&&) -> DefaultFrameManager& = delete;

  auto acquireFrame() -> std::variant<Frame*, ImageAcquireResult> override;

  [[nodiscard]] auto getFrames() const -> const std::vector<std::unique_ptr<Frame>>& override;

private:
  RenderContextConfig renderConfig;
  std::shared_ptr<CommandBufferManager> commandBufferManager;
  std::shared_ptr<Device> device;
  std::shared_ptr<Swapchain> swapchain;
  std::shared_ptr<IEventQueue> eventQueue;
  std::shared_ptr<BufferRegistry> bufferRegistry;
  std::shared_ptr<FrameState> frameState;
  std::shared_ptr<IDebugManager> debugManager;

  size_t currentFrame;
  std::vector<std::unique_ptr<Frame>> frames;

  MapKey imageKeygen;
  HandleGenerator<ManagedImage> imageHandleGenerator;

  auto handleSwapchainResized(const std::shared_ptr<SwapchainResized>& event) -> void;
};

}
