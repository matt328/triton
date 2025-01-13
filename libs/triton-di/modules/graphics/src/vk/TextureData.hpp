#pragma once

#include "mem/Image.hpp"
namespace tr {

struct TextureData {
  std::unique_ptr<Image> image;
  vk::ImageView imageView;
  vk::Sampler sampler;
  vk::ImageLayout layout;
};

}
