#include "ImmediateTransferContext.hpp"

#include "Vulkan.hpp"
#include "gfx/QueueTypes.hpp"
#include "task/CommandBufferManager.hpp"

namespace tr {

   ImmediateTransferContext::ImmediateTransferContext(
       std::shared_ptr<Device> newDevice,
       std::shared_ptr<PhysicalDevice> newPhysicalDevice,
       std::shared_ptr<queue::Transfer> newTransferQueue,
       const std::shared_ptr<CommandBufferManager>& commandBufferManager,
       const std::string_view& name)
       : device{std::move(newDevice)},
         physicalDevice{std::move(newPhysicalDevice)},
         transferQueue{std::move(newTransferQueue)},
         commandBuffer(commandBufferManager->getTransferCommandBuffer()),
         tracyContext(tracy::CreateVkContext(*physicalDevice->getVkPhysicalDevice(),
                                             *device->getVkDevice(),
                                             *transferQueue->getQueue(),
                                             **commandBuffer,
                                             nullptr,
                                             nullptr)),
         fence(std::make_unique<vk::raii::Fence>(
             device->getVkDevice().createFence(vk::FenceCreateInfo{}))) {

      tracyContext->Name(name.data(), name.length());
   }

   ImmediateTransferContext::~ImmediateTransferContext() {
      TracyVkDestroy(tracyContext);
   }

   void ImmediateTransferContext::submit(
       std::function<void(vk::raii::CommandBuffer& cmd)>&& fn) const {
      ZoneNamedN(immediateContextZone, "Immediate Transfer", true);
      constexpr vk::CommandBufferBeginInfo cmdBeginInfo{
          .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};

      commandBuffer->begin(cmdBeginInfo);
      {
         TracyVkZone(tracyContext, **commandBuffer, "Immediate Transfer");
         fn(*commandBuffer);
         TracyVkCollect(tracyContext, **commandBuffer);
      }
      commandBuffer->end();

      const auto submitInfo =
          vk::SubmitInfo{.commandBufferCount = 1, .pCommandBuffers = &**commandBuffer};

      transferQueue->getQueue().submit(submitInfo, **fence);

      if (const auto result = device->getVkDevice().waitForFences(**fence, 1u, UINT64_MAX);
          result != vk::Result::eSuccess) {
         Log.warn("Timeout waiting for fence during immediate submit");
      }
      device->getVkDevice().resetFences(**fence);
      commandBuffer->reset();
   }
}