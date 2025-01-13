#pragma once

#include "cm/Handles.hpp"
#include "as/Model.hpp"
#include "vk/ResourceManagerHandles.hpp"
#include "vk/TextureData.hpp"
#include <vulkan/vulkan_handles.hpp>

namespace tr {

class VkResourceManager;

constexpr auto MaxTextureCount = 1024;

struct Texture {
  vk::Image image;
  vk::ImageView imageView;
  vk::Sampler sampler;
};

class TextureBufferManager {
public:
  explicit TextureBufferManager(VkResourceManager* newResourceManager);
  ~TextureBufferManager() = default;

  TextureBufferManager(const TextureBufferManager&) = delete;
  TextureBufferManager(TextureBufferManager&&) = delete;
  auto operator=(const TextureBufferManager&) -> TextureBufferManager& = delete;
  auto operator=(TextureBufferManager&&) -> TextureBufferManager& = delete;

  /// TextureHandle returned here needs to be the position at which we've added this texture's
  /// Descriptor to the DescriptorBuffer
  auto addTexture(const as::ImageData& imageData, std::string_view name) -> TextureHandle;
  auto removeTexture(TextureHandle handle) -> void;

  [[nodiscard]] auto getDescriptorBufferHandle() const -> BufferHandle;

private:
  VkResourceManager* resourceManager;
  BufferHandle descriptorBuffer;

  std::unordered_map<size_t, TextureData> textureDataMap{};
  std::stack<size_t> freeList{};
};

}
