#pragma once

#include "bk/HandleGenerator.hpp"
#include "bk/Rando.hpp"
#include "gfx/IFrameManager.hpp"
#include "gfx/RenderContextConfig.hpp"
#include "api/fx/IEventBus.hpp"

namespace tr {

class CommandBufferManager;
class Swapchain;
class VkResourceManager;
class ImageRegistry;
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
                               std::shared_ptr<ImageRegistry> newImageRegistry,
                               std::shared_ptr<BufferRegistry> newBufferRegistry);
  ~DefaultFrameManager() override;

  DefaultFrameManager(const DefaultFrameManager&) = delete;
  DefaultFrameManager(DefaultFrameManager&&) = delete;
  auto operator=(const DefaultFrameManager&) -> DefaultFrameManager& = delete;
  auto operator=(DefaultFrameManager&&) -> DefaultFrameManager& = delete;

  auto acquireFrame() -> std::variant<Frame*, ImageAcquireResult> override;

  [[nodiscard]] auto getFrames() const -> const std::vector<std::unique_ptr<Frame>>& override;

  auto createPerFrameBuffer() -> LogicalBufferHandle override;

  auto registerBufferRequest(BufferKey& bufferKey) -> Handle<BufferWrapper> override;

  auto registerImageRequest(const ImageRequest& request) -> Handle<ManagedImage> override;

private:
  RenderContextConfig renderConfig;
  std::shared_ptr<CommandBufferManager> commandBufferManager;
  std::shared_ptr<Device> device;
  std::shared_ptr<Swapchain> swapchain;
  std::shared_ptr<VkResourceManager> resourceManager;
  std::shared_ptr<IEventBus> eventBus;
  std::shared_ptr<ImageRegistry> imageRegistry;
  std::shared_ptr<BufferRegistry> bufferRegistry;

  size_t currentFrame;
  std::vector<std::unique_ptr<Frame>> frames;

  MapKey imageKeygen;
  MapKey bufferKeygen;

  HandleGenerator<ManagedImage> imageHandleGenerator;
  HandleGenerator<BufferWrapper> bufferHandleGenerator;

  auto handleSwapchainResized(const SwapchainResized& event) -> void;
};

}
