#pragma once

#include "CommandBufferManager.hpp"
#include "IFrameManager.hpp"

namespace tr::gfx {
   struct RenderContextConfig;
}
namespace tr::gfx::task {

   class DefaultFrameManager final : public IFrameManager {
    public:
      explicit DefaultFrameManager(const RenderContextConfig& rendererConfig,
                                   std::shared_ptr<CommandBufferManager> newCommandBufferManager,
                                   std::shared_ptr<VkResourceManager> newResourceManager,
                                   std::shared_ptr<queue::Graphics> newGraphicsQueue);
      ~DefaultFrameManager() override;

      DefaultFrameManager(const DefaultFrameManager&) = delete;
      DefaultFrameManager(DefaultFrameManager&&) = delete;
      auto operator=(const DefaultFrameManager&) -> DefaultFrameManager& = delete;
      auto operator=(DefaultFrameManager&&) -> DefaultFrameManager& = delete;

      auto acquireFrame() -> Frame& override;
      auto submitFrame(Frame& frame) -> void override;

    private:
      size_t currentFrame;
      std::shared_ptr<CommandBufferManager> commandBufferManager;
      std::shared_ptr<VkResourceManager> resourceManager;
      std::shared_ptr<queue::Graphics> graphicsQueue;

      std::vector<std::unique_ptr<Frame>> frames;
   };

}
