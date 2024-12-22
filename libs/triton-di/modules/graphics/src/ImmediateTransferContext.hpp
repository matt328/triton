#pragma once

#include "gfx/QueueTypes.hpp"
#include "task/CommandBufferManager.hpp"
#include "vk/Device.hpp"
#include "vk/PhysicalDevice.hpp"

namespace vk::raii {
   class Device;
   class CommandBuffer;
   class CommandPool;
   class Fence;
   class Queue;
}

namespace tr {

   class ImmediateTransferContext {
    public:
      ImmediateTransferContext(std::shared_ptr<Device> newDevice,
                               std::shared_ptr<PhysicalDevice> physicalDevice,
                               std::shared_ptr<queue::Transfer> newTransferQueue,
                               const std::shared_ptr<CommandBufferManager>& commandBufferManager,
                               const std::string_view& name = "unnamed immediate context");

      ~ImmediateTransferContext();

      ImmediateTransferContext(const ImmediateTransferContext&) = delete;
      ImmediateTransferContext(ImmediateTransferContext&&) = delete;
      auto operator=(const ImmediateTransferContext&) -> ImmediateTransferContext& = delete;
      auto operator=(ImmediateTransferContext&&) -> ImmediateTransferContext& = delete;

      void submit(std::function<void(vk::raii::CommandBuffer& cmd)>&& fn) const;

    private:
      std::shared_ptr<Device> device;
      std::shared_ptr<PhysicalDevice> physicalDevice;
      std::shared_ptr<queue::Transfer> transferQueue;

      CommandBufferPtr commandBuffer;
      tracy::VkCtx* tracyContext;
      std::unique_ptr<vk::raii::Fence> fence = nullptr;
   };
}