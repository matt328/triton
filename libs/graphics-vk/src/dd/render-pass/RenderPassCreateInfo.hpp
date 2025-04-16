#pragma once

#include "bk/Handle.hpp"
#include "img/ManagedImage.hpp"

namespace tr {

struct AttachmentRequest {
  Handle<ManagedImage> logicalImage;
  vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear;
  vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore;
};

struct RenderPassCreateInfo {
  std::vector<AttachmentRequest> colorAttachments;
  std::optional<AttachmentRequest> depthAttachment = std::nullopt;
  std::optional<std::string> debugName = std::nullopt;
};

}
