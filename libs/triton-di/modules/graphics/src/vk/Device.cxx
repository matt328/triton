#include "Device.hpp"

namespace tr::gfx {

   Device::Device(const std::shared_ptr<PhysicalDevice>& physicalDevice) {
      device = physicalDevice->createDevice();
      queueFamilyIndices = physicalDevice->getQueueFamilyIndices();
   }

   Device::~Device() {
      Log.trace("Destroying Device");
   }

   auto Device::createGraphicsQueue() const -> std::unique_ptr<vk::raii::Queue> {
      return std::make_unique<vk::raii::Queue>(
          device->getQueue(queueFamilyIndices.graphicsFamily.value(), 0));
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

}