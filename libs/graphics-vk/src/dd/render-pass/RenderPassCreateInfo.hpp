#pragma once

namespace tr {

struct AttachmentInfo {
  std::string logicalName;
  vk::Format format;
  vk::Extent2D extent;
  vk::ImageUsageFlags usage;
  vk::ImageAspectFlags aspect;
  vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined;
  vk::ImageLayout finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
  vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear;
  vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore;
};

struct RenderPassCreateInfo {
  std::optional<AttachmentInfo> colorAttachment = std::nullopt;
  std::optional<AttachmentInfo> depthAttachment = std::nullopt;
};

}
