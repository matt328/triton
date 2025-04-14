#pragma once

#include "bk/Rando.hpp"
#include "img/ImageWrapper.hpp"
namespace tr {

using ImageHandle = size_t;

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
};

class Allocator;
class IDebugManager;

class ImageManager {
public:
  ImageManager(std::shared_ptr<Allocator> newAllocator,
               std::shared_ptr<IDebugManager> newDebugManager);
  ~ImageManager() = default;

  ImageManager(const ImageManager&) = delete;
  ImageManager(ImageManager&&) = delete;
  auto operator=(const ImageManager&) -> ImageManager& = delete;
  auto operator=(ImageManager&&) -> ImageManager& = delete;

  auto createImage(const ImageUsageProfile& profile) -> ImageHandle;
  auto getImage(ImageHandle imageHandle) -> ManagedImage;

private:
  std::shared_ptr<Allocator> allocator;
  std::shared_ptr<IDebugManager> debugManager;

  MapKey imageKey;
  std::unordered_map<ImageHandle, std::unique_ptr<ManagedImage>> imageMap;
};

}
