#pragma once

#include "cm/Handles.hpp"
#include "as/Model.hpp"
#include "vk/VkResourceManager.hpp"

namespace tr {

constexpr auto MaxTextureCount = 1024;

class TextureManager {
public:
  explicit TextureManager(VkResourceManager* newResourceManager);
  ~TextureManager() = default;

  TextureManager(const TextureManager&) = delete;
  TextureManager(TextureManager&&) = delete;
  auto operator=(const TextureManager&) -> TextureManager& = delete;
  auto operator=(TextureManager&&) -> TextureManager& = delete;

  [[nodiscard]] auto addTexture(const as::ImageData& imageData, std::string_view name)
      -> TextureHandle;

  [[nodiscard]] auto isDirty() const -> bool;

  [[nodiscard]] auto getDescriptorImageInfoList() -> const std::vector<vk::DescriptorImageInfo>&;

private:
  VkResourceManager* resourceManager;

  bool dirty{};
  std::vector<vk::DescriptorImageInfo> imageInfoList;
  std::vector<TextureData> textureDataList;
};

}
