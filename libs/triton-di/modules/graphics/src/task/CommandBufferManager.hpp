#pragma once
#include <vk/Device.hpp>

namespace tr::gfx {

   struct RendererConfig {
      uint32_t framesInFlight;
   };

   class CommandBufferManager {
    public:
      CommandBufferManager(std::shared_ptr<Device> newDevice, const RendererConfig& rendererConfig);
      ~CommandBufferManager();

      CommandBufferManager(const CommandBufferManager&) = delete;
      CommandBufferManager(CommandBufferManager&&) = delete;
      auto operator=(const CommandBufferManager&) -> CommandBufferManager& = delete;
      auto operator=(CommandBufferManager&&) -> CommandBufferManager& = delete;

    private:
      std::shared_ptr<Device> device;
      uint32_t framesInFlight;
   };

}