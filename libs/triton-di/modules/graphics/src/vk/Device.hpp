#pragma once
#include "PhysicalDevice.hpp"

namespace tr::gfx {

   class Device {
    public:
      explicit Device(const std::shared_ptr<PhysicalDevice>& physicalDevice);
      ~Device();

      auto operator=(const Device&) -> Device& = delete;
      auto operator=(Device&&) -> Device& = delete;
      Device(const Device&) = delete;
      Device(Device&&) = delete;

      [[nodiscard]] auto createGraphicsQueue() const -> std::unique_ptr<vk::raii::Queue>;
      [[nodiscard]] auto createPresentQueue() const -> std::unique_ptr<vk::raii::Queue>;
      [[nodiscard]] auto createTransferQueue() const -> std::unique_ptr<vk::raii::Queue>;
      [[nodiscard]] auto createComputeQueue() const -> std::unique_ptr<vk::raii::Queue>;

      [[nodiscard]] auto createDescriptorPool(const vk::DescriptorPoolCreateInfo& info) const
          -> vk::raii::DescriptorPool;

      [[nodiscard]] auto getVkDevice() const -> vk::raii::Device&;

      [[nodiscard]] auto createSwapchain(const vk::SwapchainCreateInfoKHR& info) const
          -> std::unique_ptr<vk::raii::SwapchainKHR>;

    private:
      std::unique_ptr<vk::raii::Device> device;
      QueueFamilyIndices queueFamilyIndices;
   };

}