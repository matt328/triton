#pragma once

#include "CommandBufferManager.hpp"
#include "IFrameManager.hpp"

namespace tr::gfx {
   class Swapchain;
   struct RenderContextConfig;
}
namespace tr::gfx::task {

   class DefaultFrameManager final : public IFrameManager {
    public:
      explicit DefaultFrameManager(const RenderContextConfig& rendererConfig,
                                   std::shared_ptr<CommandBufferManager> newCommandBufferManager,
                                   std::shared_ptr<Device> newDevice,
                                   std::shared_ptr<Swapchain> newSwapchain,
                                   std::shared_ptr<VkResourceManager> newResourceManager);
      ~DefaultFrameManager() override;

      DefaultFrameManager(const DefaultFrameManager&) = delete;
      DefaultFrameManager(DefaultFrameManager&&) = delete;
      auto operator=(const DefaultFrameManager&) -> DefaultFrameManager& = delete;
      auto operator=(DefaultFrameManager&&) -> DefaultFrameManager& = delete;

      auto acquireFrame()
          -> std::variant<std::reference_wrapper<Frame>, ImageAcquireResult> override;

    private:
      size_t currentFrame;
      std::shared_ptr<CommandBufferManager> commandBufferManager;
      std::shared_ptr<Device> device;
      std::shared_ptr<Swapchain> swapchain;
      std::shared_ptr<VkResourceManager> resourceManager;

      std::vector<std::unique_ptr<Frame>> frames;
   };

}
