#pragma once
#include "vk/core/PhysicalDevice.hpp"

namespace tr {

class Device {
public:
  explicit Device(std::shared_ptr<PhysicalDevice> newPhysicalDevice);
  ~Device();

  auto operator=(const Device&) -> Device& = delete;
  auto operator=(Device&&) -> Device& = delete;
  Device(const Device&) = delete;
  Device(Device&&) = delete;

  [[nodiscard]] auto createGraphicsQueue() const -> std::shared_ptr<vk::raii::Queue>;
  [[nodiscard]] auto createPresentQueue() const -> std::unique_ptr<vk::raii::Queue>;
  [[nodiscard]] auto createTransferQueue() const -> std::unique_ptr<vk::raii::Queue>;
  [[nodiscard]] auto createComputeQueue() const -> std::unique_ptr<vk::raii::Queue>;

  [[nodiscard]] auto getGraphicsQueueFamily() const -> uint32_t;
  [[nodiscard]] auto getPresentQueueFamily() const -> uint32_t;
  [[nodiscard]] auto getTransferQueueFamily() const -> uint32_t;
  [[nodiscard]] auto getComputeQueueFamily() const -> uint32_t;

  [[nodiscard]] auto createDescriptorPool(const vk::DescriptorPoolCreateInfo& info) const
      -> vk::raii::DescriptorPool;

  [[nodiscard]] auto getVkDevice() const -> vk::raii::Device&;

  [[nodiscard]] auto createSwapchain(const vk::SwapchainCreateInfoKHR& info) const
      -> std::unique_ptr<vk::raii::SwapchainKHR>;

  auto waitIdle() const -> void;

private:
  std::unique_ptr<vk::raii::Device> device;
  std::shared_ptr<PhysicalDevice> physicalDevice;
  QueueFamilyIndices queueFamilyIndices;
};

}
