#pragma once

namespace tr {

struct AttachmentRequest {
  std::string logicalName;
  vk::Format format;
  vk::Extent2D extent;
  vk::ImageUsageFlags usage;
  vk::ImageAspectFlags aspect;
  vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear;
  vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore;
};

struct RenderPassCreateInfo {
  std::vector<AttachmentRequest> colorAttachments;
  std::optional<AttachmentRequest> depthAttachment = std::nullopt;
  std::optional<std::string> debugName = std::nullopt;
};

}
