#pragma once

#include "bk/HandleGenerator.hpp"
#include "img/ImageRequest.hpp"
#include "img/ManagedImage.hpp"

namespace tr {

class ImageManager;

class ImageRegistry {
public:
  explicit ImageRegistry(std::shared_ptr<ImageManager> newImageManager);
  ~ImageRegistry();

  ImageRegistry(const ImageRegistry&) = delete;
  ImageRegistry(ImageRegistry&&) = delete;
  auto operator=(const ImageRegistry&) -> ImageRegistry& = delete;
  auto operator=(ImageRegistry&&) -> ImageRegistry& = delete;

  auto getOrCreate(const ImageKey& key) -> Handle<ManagedImage>;

private:
  std::shared_ptr<ImageManager> imageManager;

  HandleGenerator<ManagedImage> generator;
  std::unordered_map<ImageKey, Handle<ManagedImage>> imageHandles;
};

}
