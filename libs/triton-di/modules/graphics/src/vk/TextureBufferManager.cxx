#include "TextureBufferManager.hpp"
#include "vk/MeshBufferManager.hpp"
#include "vk/VkResourceManager.hpp"

namespace tr {

TextureBufferManager::TextureBufferManager(VkResourceManager* newResourceManager)
    : resourceManager{newResourceManager} {

  for (int i = MaxTextureCount - 1; i >= 0; --i) {
    freeList.push(i);
  }

  descriptorBuffer = resourceManager->createDescriptorBuffer(MaxTextureCount, "Buffer-Texture");
}

auto TextureBufferManager::addTexture([[maybe_unused]] const as::ImageData& imageData,
                                      std::string_view name) -> TextureHandle {
  if (freeList.empty()) {
    throw std::runtime_error("Ran out of texture descriptor slots");
  }

  size_t index = freeList.top();
  freeList.pop();

  // Upload the image and place the handles in the map
  textureDataMap.emplace(index, resourceManager->getTextureData(imageData, name));

  // add DescriptorImageInfo to the buffer at slot index with getDescriptorEXT
  const auto descriptorImageInfo =
      vk::DescriptorImageInfo{.sampler = textureDataMap.at(index).sampler,
                              .imageView = textureDataMap.at(index).imageView,
                              .imageLayout = textureDataMap.at(index).layout};

  resourceManager->addDescriptorToBuffer(descriptorBuffer, descriptorImageInfo, index);

  return index;
}

auto TextureBufferManager::removeTexture([[maybe_unused]] TextureHandle handle) -> void {
}

auto TextureBufferManager::getDescriptorBufferHandle() const -> BufferHandle {
  return descriptorBuffer;
}

}
