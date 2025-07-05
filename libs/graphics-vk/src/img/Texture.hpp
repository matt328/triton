#pragma once

namespace tr {

struct Texture {
  vk::ImageView view;
  vk::Sampler sampler;
};

}
