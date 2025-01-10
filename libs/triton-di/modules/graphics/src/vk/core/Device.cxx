#include "Device.hpp"

namespace tr {

Device::Device(std::shared_ptr<PhysicalDevice> newPhysicalDevice)
    : physicalDevice{std::move(newPhysicalDevice)} {
  device = physicalDevice->createDevice();
  queueFamilyIndices = physicalDevice->getQueueFamilyIndices();
}

Device::~Device() {
  Log.trace("Destroying Device");
}

auto Device::createGraphicsQueue() const -> std::shared_ptr<vk::raii::Queue> {
  return std::make_shared<vk::raii::Queue>(
      device->getQueue(queueFamilyIndices.graphicsFamily.value(), 0));
}

auto Device::getGraphicsQueueFamily() const -> uint32_t {
  return queueFamilyIndices.graphicsFamily.value();
}

auto Device::getPresentQueueFamily() const -> uint32_t {
  return queueFamilyIndices.presentFamily.value();
}

auto Device::getTransferQueueFamily() const -> uint32_t {
  return queueFamilyIndices.transferFamily.value();
}

auto Device::getComputeQueueFamily() const -> uint32_t {
  return queueFamilyIndices.computeFamily.value();
}

auto Device::createPresentQueue() const -> std::unique_ptr<vk::raii::Queue> {
  return std::make_unique<vk::raii::Queue>(
      device->getQueue(queueFamilyIndices.presentFamily.value(), 0));
}

auto Device::createTransferQueue() const -> std::unique_ptr<vk::raii::Queue> {
  return std::make_unique<vk::raii::Queue>(
      device->getQueue(queueFamilyIndices.transferFamily.value(), 0));
}

auto Device::createComputeQueue() const -> std::unique_ptr<vk::raii::Queue> {
  return std::make_unique<vk::raii::Queue>(
      device->getQueue(queueFamilyIndices.computeFamily.value(), 0));
}

auto Device::createDescriptorPool(const vk::DescriptorPoolCreateInfo& info) const
    -> vk::raii::DescriptorPool {
  return device->createDescriptorPool(info);
}

auto Device::getVkDevice() const -> vk::raii::Device& {
  return *device;
}

auto Device::createSwapchain(const vk::SwapchainCreateInfoKHR& info) const
    -> std::unique_ptr<vk::raii::SwapchainKHR> {
  return std::make_unique<vk::raii::SwapchainKHR>(device->createSwapchainKHR(info));
}
auto Device::waitIdle() const -> void {
  device->waitIdle();
}

}
