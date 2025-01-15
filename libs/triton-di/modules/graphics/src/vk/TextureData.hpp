#pragma once

#include "mem/Image.hpp"
namespace tr {

struct TextureData {
  std::unique_ptr<Image> image;
  std::unique_ptr<vk::raii::ImageView> imageView;
  std::unique_ptr<vk::raii::Sampler> sampler;
  vk::ImageLayout layout;
};

}
