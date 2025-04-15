#pragma once

#include "bk/Handle.hpp"
#include "img/ManagedImage.hpp"

namespace tr {

struct RenderPassConfig {
  std::optional<vk::RenderingAttachmentInfo> colorAttachmentInfo;
  std::optional<vk::RenderingAttachmentInfo> depthAttachmentInfo;
  std::optional<Handle<ManagedImage>> colorHandle;
  std::optional<Handle<ManagedImage>> depthHandle;
  vk::Extent2D extent;
};

}
