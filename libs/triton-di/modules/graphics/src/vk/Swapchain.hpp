#pragma once
#include "Device.hpp"
#include "PhysicalDevice.hpp"

namespace tr::gfx {

   class Swapchain {
    public:
      Swapchain(std::shared_ptr<PhysicalDevice> newPhysicalDevice,
                std::shared_ptr<Device> newDevice,
                std::shared_ptr<Surface> newSurface);
      ~Swapchain();

      Swapchain(const Swapchain&) = delete;
      Swapchain(Swapchain&&) = delete;
      auto operator=(const Swapchain&) -> Swapchain& = delete;
      auto operator=(Swapchain&&) -> Swapchain& = delete;

    private:
      std::shared_ptr<PhysicalDevice> physicalDevice;
      std::shared_ptr<Device> device;
      std::shared_ptr<Surface> surface;

      std::unique_ptr<vk::raii::SwapchainKHR> oldSwapchain = nullptr;
      std::unique_ptr<vk::raii::SwapchainKHR> swapchain = nullptr;

      std::vector<vk::Image> swapchainImages;
      std::vector<vk::raii::ImageView> swapchainImageViews;

      [[nodiscard]] static auto choosePresentMode(
          const std::vector<vk::PresentModeKHR>& availablePresentModes) -> vk::PresentModeKHR;

      [[nodiscard]] static auto chooseSurfaceFormat(
          const std::vector<vk::SurfaceFormatKHR>& availableFormats) -> vk::SurfaceFormatKHR;

      [[nodiscard]] static auto chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities,
                                                 const std::pair<uint32_t, uint32_t>& windowSize)
          -> vk::Extent2D;
   };

}