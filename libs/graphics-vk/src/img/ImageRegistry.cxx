#include "ImageRegistry.hpp"

namespace tr {

ImageRegistry::ImageRegistry(std::shared_ptr<ImageManager> newImageManager)
    : imageManager{std::move(newImageManager)} {
}

ImageRegistry::~ImageRegistry() {
}

auto ImageRegistry::getOrCreate(const ImageKey& key) -> Handle<ManagedImage> {
  if (imageHandles.contains(key)) {
    return imageHandles.at(key);
  }
}

auto ImageRegistry::swapchainResized([[maybe_unused]] vk::Extent2D swapchainSize,
                                     [[maybe_unused]] float renderScale) -> void {
}

}
