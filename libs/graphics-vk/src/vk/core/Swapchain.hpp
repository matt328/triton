#pragma once
#include "Device.hpp"
#include "PhysicalDevice.hpp"

namespace tr {

enum class ImageAcquireResult : uint8_t {
  Success,
  NeedsResize,
  Error
};

class IEventQueue;

class Swapchain {
public:
  Swapchain(std::shared_ptr<PhysicalDevice> newPhysicalDevice,
            std::shared_ptr<Device> newDevice,
            std::shared_ptr<Surface> newSurface,
            std::shared_ptr<IEventQueue> newEventQueue,
            std::shared_ptr<IDebugManager> newDebugManager);
  ~Swapchain();

  Swapchain(const Swapchain&) = delete;
  Swapchain(Swapchain&&) = delete;
  auto operator=(const Swapchain&) -> Swapchain& = delete;
  auto operator=(Swapchain&&) -> Swapchain& = delete;

  [[nodiscard]] auto getImageFormat() const -> vk::Format;
  [[nodiscard]] auto getImageExtent() const -> vk::Extent2D;
  [[nodiscard]] auto getSwapchain() const noexcept -> vk::SwapchainKHR;
  [[nodiscard]] auto getSwapchainImage(uint32_t imageIndex) const -> vk::Image;
  [[nodiscard]] auto getSwapchainImageView(uint32_t imageIndex) const -> vk::ImageView;
  [[nodiscard]] auto getDepthFormat() const -> vk::Format;
  [[nodiscard]] auto getImages() const -> const std::vector<vk::Image>&;
  [[nodiscard]] auto getImageSemaphore(uint32_t imageIndex) const -> vk::Semaphore;

  [[nodiscard]] auto acquireNextImage(const vk::Semaphore& semaphore) const
      -> std::variant<uint32_t, ImageAcquireResult>;

  auto recreate() -> void;

private:
  std::shared_ptr<PhysicalDevice> physicalDevice;
  std::shared_ptr<Device> device;
  std::shared_ptr<Surface> surface;
  std::shared_ptr<IEventQueue> eventQueue;
  std::shared_ptr<IDebugManager> debugManager;

  std::unique_ptr<vk::raii::SwapchainKHR> oldSwapchain = nullptr;
  std::unique_ptr<vk::raii::SwapchainKHR> swapchain = nullptr;

  std::vector<vk::Image> swapchainImages;
  std::vector<vk::raii::ImageView> swapchainImageViews;
  std::vector<vk::raii::Semaphore> imageSemaphores;

  vk::Format swapchainImageFormat;
  vk::Extent2D swapchainExtent;

  auto createSwapchain() -> void;

  [[nodiscard]] static auto choosePresentMode(
      const std::vector<vk::PresentModeKHR>& availablePresentModes) -> vk::PresentModeKHR;

  [[nodiscard]] static auto chooseSurfaceFormat(
      const std::vector<vk::SurfaceFormatKHR>& availableFormats) -> vk::SurfaceFormatKHR;

  [[nodiscard]] static auto chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities,
                                             const vk::Extent2D& windowSize) -> vk::Extent2D;
};

}
