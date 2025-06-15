#pragma once

#include "bk/Handle.hpp"
#include "bk/HandleGenerator.hpp"
#include "bk/Rando.hpp"
#include "img/ImageRequest.hpp"
#include "img/ManagedImage.hpp"
namespace tr {

enum class ImageLifetime : uint8_t {
  PerFrame = 0,
  Persistent
};

struct ImageUsageProfile {
  vk::Format format;
  vk::ImageUsageFlags usage;
  vk::ImageAspectFlags aspectMask;
  vk::Extent2D extent;
  uint32_t mipLevels = 1;
  uint32_t layers = 1;
  ImageLifetime lifetime = ImageLifetime::PerFrame;
  std::optional<std::string> debugName = std::nullopt;
};

struct ImageMetadata {
  vk::Format format;
  vk::Extent2D extent;
  vk::ImageUsageFlags imageUsage;
};

class Allocator;
class IDebugManager;
class Device;
class IFrameManager;
class Swapchain;

class ImageManager {
public:
  ImageManager(std::shared_ptr<Allocator> newAllocator,
               std::shared_ptr<IDebugManager> newDebugManager,
               std::shared_ptr<Device> newDevice,
               std::shared_ptr<IFrameManager> newFrameManager,
               std::shared_ptr<Swapchain> newSwapchain);
  ~ImageManager();

  ImageManager(const ImageManager&) = delete;
  ImageManager(ImageManager&&) = delete;
  auto operator=(const ImageManager&) -> ImageManager& = delete;
  auto operator=(ImageManager&&) -> ImageManager& = delete;

  auto createImage(ImageRequest request) -> Handle<ManagedImage>;
  auto createPerFrameImage(ImageRequest request) -> LogicalHandle<ManagedImage>;
  auto getImage(Handle<ManagedImage> imageHandle) -> ManagedImage&;
  auto getImageMetadata(LogicalHandle<ManagedImage> logicalHandle) -> ImageMetadata;
  auto getImageMetadata(Handle<ManagedImage> handle) -> ImageMetadata;
  auto getSwapchainImageHandle() const -> LogicalHandle<ManagedImage>;
  auto registerSwapchainImage(uint32_t index) -> Handle<ManagedImage>;

private:
  std::shared_ptr<Allocator> allocator;
  std::shared_ptr<IDebugManager> debugManager;
  std::shared_ptr<Device> device;
  std::shared_ptr<IFrameManager> frameManager;
  std::shared_ptr<Swapchain> swapchain;

  HandleGenerator<ManagedImage> generator;
  std::unordered_map<Handle<ManagedImage>, std::unique_ptr<ManagedImage>> imageMap;
  LogicalHandle<ManagedImage> swapchainLogicalHandle;
  std::unordered_map<Handle<ManagedImage>, uint32_t> handleToSwapchainIndex;

  auto registerSwapchainImages() -> void;
};

}
