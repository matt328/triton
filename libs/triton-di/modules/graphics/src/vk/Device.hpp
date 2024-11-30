#pragma once
#include "PhysicalDevice.hpp"
#include "gfx/QueueTypes.hpp"

namespace tr::gfx {

   class Device {
    public:
      explicit Device(const std::shared_ptr<PhysicalDevice>& physicalDevice);
      ~Device();

      auto operator=(const Device&) -> Device& = delete;
      auto operator=(Device&&) -> Device& = delete;
      Device(const Device&) = delete;
      Device(Device&&) = delete;

      [[nodiscard]] auto createGraphicsQueue() const -> std::shared_ptr<queue::Graphics>;
      [[nodiscard]] auto createPresentQueue() const -> std::shared_ptr<queue::Present>;
      [[nodiscard]] auto createTransferQueue() const -> std::shared_ptr<queue::Transfer>;
      [[nodiscard]] auto createComputeQueue() const -> std::shared_ptr<queue::Compute>;

      [[nodiscard]] auto createDescriptorPool(const vk::DescriptorPoolCreateInfo& info) const
          -> vk::raii::DescriptorPool;

      [[nodiscard]] auto getVkDevice() const -> vk::raii::Device&;

      [[nodiscard]] auto createSwapchain(const vk::SwapchainCreateInfoKHR& info) const -> std::unique_ptr<vk::raii::SwapchainKHR>;

    private:
      std::unique_ptr<vk::raii::Device> device;
      QueueFamilyIndices queueFamilyIndices;
   };

}