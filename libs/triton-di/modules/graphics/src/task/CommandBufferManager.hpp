#pragma once
#include "CommandBufferPool.hpp"

#include <vk/Device.hpp>

namespace tr::gfx {
   struct RenderContextConfig;
   namespace queue {
      class Compute;
      class Transfer;
      class Graphics;
   }

   using CommandBufferPtr =
       std::unique_ptr<vk::raii::CommandBuffer, CommandBufferPool::ReturnToPoolDeleter>;

   class CommandBufferManager {
    public:
      CommandBufferManager(std::shared_ptr<Device> newDevice,
                           const std::shared_ptr<queue::Graphics>& graphicsQueue,
                           const std::shared_ptr<queue::Transfer>& transferQueue,
                           const std::shared_ptr<queue::Compute>& computeQueue,
                           const RenderContextConfig& rendererConfig);
      ~CommandBufferManager();

      CommandBufferManager(const CommandBufferManager&) = delete;
      CommandBufferManager(CommandBufferManager&&) = delete;
      auto operator=(const CommandBufferManager&) -> CommandBufferManager& = delete;
      auto operator=(CommandBufferManager&&) -> CommandBufferManager& = delete;

      auto getCommandBuffer(uint32_t frameIndex, uint32_t queueFamily) -> CommandBufferPtr;

    private:
      std::shared_ptr<Device> device;
      uint32_t framesInFlight;

      std::unordered_map<uint64_t, std::unique_ptr<CommandBufferPool>> commandPools;

      static auto getKey(uint32_t frameIndex, uint32_t queueIndex) -> uint64_t;
   };

}
