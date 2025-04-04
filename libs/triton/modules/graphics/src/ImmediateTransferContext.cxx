#include "ImmediateTransferContext.hpp"

#include "gfx/QueueTypes.hpp"
#include "vk/CommandBufferManager.hpp"

namespace tr {

ImmediateTransferContext::ImmediateTransferContext(
    std::shared_ptr<Device> newDevice,
    std::shared_ptr<PhysicalDevice> newPhysicalDevice,
    std::shared_ptr<queue::Transfer> newTransferQueue,
    std::shared_ptr<CommandBufferManager> newCommandBufferManager,
    [[maybe_unused]] const std::string_view& name)
    : device{std::move(newDevice)},
      physicalDevice{std::move(newPhysicalDevice)},
      transferQueue{std::move(newTransferQueue)},
      commandBufferManager{std::move(newCommandBufferManager)},
      fence(std::make_unique<vk::raii::Fence>(
          device->getVkDevice().createFence(vk::FenceCreateInfo{}))) {

  // auto& commandBuffer = commandBufferManager->getCommandBuffer(commandBufferHandle);

  // tracyContext = tracy::CreateVkContext(*physicalDevice->getVkPhysicalDevice(),
  //                                       *device->getVkDevice(),
  //                                       *transferQueue->getQueue(),
  //                                       *commandBuffer,
  //                                       nullptr,
  //                                       nullptr),

  // tracyContext->Name(name.data(), name.length());
}

ImmediateTransferContext::~ImmediateTransferContext() {
  // TracyVkDestroy(tracyContext);
}
void ImmediateTransferContext::submit(
    std::function<void(vk::raii::CommandBuffer& cmd)>&& fn) const {
  TracyMessageL("ImmediateContext Begin");

  ZoneNamedN(immediateContextZone, "Immediate Transfer", true);
  constexpr vk::CommandBufferBeginInfo cmdBeginInfo{
      .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};

  auto commandBuffer = commandBufferManager->getTransferCommandBuffer();

  commandBuffer.begin(cmdBeginInfo);
  {
    // TracyVkZone(tracyContext, *commandBuffer, "Immediate Transfer");
    fn(commandBuffer);
    // TracyVkCollect(tracyContext, *commandBuffer);
  }
  commandBuffer.end();

  const auto submitInfo =
      vk::SubmitInfo{.commandBufferCount = 1, .pCommandBuffers = &*commandBuffer};

  transferQueue->getQueue().submit(submitInfo, **fence);

  if (const auto result = device->getVkDevice().waitForFences(**fence, 1u, UINT64_MAX);
      result != vk::Result::eSuccess) {
    Log.warn("Timeout waiting for fence during immediate submit");
  }
  device->getVkDevice().resetFences(**fence);

  TracyMessageL("Immediate Context Finished");
}

}
