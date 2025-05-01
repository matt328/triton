#include "ImageRegistry.hpp"
#include "img/ImageManager.hpp"

namespace tr {

ImageRegistry::ImageRegistry(std::shared_ptr<ImageManager> newImageManager)
    : imageManager{std::move(newImageManager)} {
}

ImageRegistry::~ImageRegistry() {
  Log.trace("Destroying ImageRegistry");
  imageHandles.clear();
}

auto ImageRegistry::getOrCreate(const ImageKey& key) -> Handle<ManagedImage> {
  if (imageHandles.contains(key)) {
    return imageHandles.at(key);
  }
  const auto handle = imageManager->createImage(key.request);
  imageHandles.emplace(key, handle);
  return handle;
}

auto ImageRegistry::swapchainResized([[maybe_unused]] vk::Extent2D swapchainSize,
                                     [[maybe_unused]] float renderScale) -> void {
}

}
