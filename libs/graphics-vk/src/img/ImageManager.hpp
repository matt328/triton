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

class Allocator;
class IDebugManager;
class Device;

class ImageManager {
public:
  ImageManager(std::shared_ptr<Allocator> newAllocator,
               std::shared_ptr<IDebugManager> newDebugManager,
               std::shared_ptr<Device> newDevice);
  ~ImageManager() = default;

  ImageManager(const ImageManager&) = delete;
  ImageManager(ImageManager&&) = delete;
  auto operator=(const ImageManager&) -> ImageManager& = delete;
  auto operator=(ImageManager&&) -> ImageManager& = delete;

  auto createImage(const ImageRequest& request) -> Handle<ManagedImage>;
  auto getImage(Handle<ManagedImage> imageHandle) -> ManagedImage&;

private:
  std::shared_ptr<Allocator> allocator;
  std::shared_ptr<IDebugManager> debugManager;
  std::shared_ptr<Device> device;

  HandleGenerator<ManagedImage> generator;
  std::unordered_map<Handle<ManagedImage>, std::unique_ptr<ManagedImage>> imageMap;
};

}
