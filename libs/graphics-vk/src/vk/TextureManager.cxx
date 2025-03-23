#include "TextureManager.hpp"

namespace tr {

TextureManager::TextureManager(VkResourceManager* newResourceManager)
    : resourceManager{newResourceManager} {
}

auto TextureManager::addTexture(const as::ImageData& imageData, std::string_view name)
    -> TextureHandle {
  const auto textureHandle = imageInfoList.size();

  textureDataList.emplace_back(resourceManager->getTextureData(imageData, name));

  imageInfoList.emplace_back(vk::DescriptorImageInfo{
      .sampler = *textureDataList.back().sampler,
      .imageView = **textureDataList.back().imageView,
      .imageLayout = textureDataList.back().layout,
  });

  dirty = true;

  return textureHandle;
}

auto TextureManager::isDirty() const -> bool {
  return dirty;
}

auto TextureManager::getDescriptorImageInfoList() -> const std::vector<vk::DescriptorImageInfo>& {
  return imageInfoList;
}

}
