#pragma once

#include "bk/Handle.hpp"
#include "img/ManagedImage.hpp"
namespace tr {

struct AttachmentConfig {
  Handle<ManagedImage> logicalImage;
  vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear;
  vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore;
  std::optional<vk::ClearValue> clearValue = std::nullopt;
};

}
