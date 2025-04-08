#pragma once

#include "dd/DrawContext.hpp"
#include "dd/LogicalImageHandle.hpp"
#include "dd/RenderConfigHandle.hpp"

namespace tr {

class DrawContextFactory;
class Frame;
class VkResourceManager;

struct RenderPassConfig {
  vk::Extent2D extent;
  std::optional<LogicalImageHandle> colorHandle;
  std::optional<LogicalImageHandle> depthHandle;
  std::optional<vk::RenderingAttachmentInfo> colorAttachmentInfo;
  std::optional<vk::RenderingAttachmentInfo> depthAttachmentInfo;
  std::string name = "Unnamed RenderPass";
};

class RenderPass {
public:
  explicit RenderPass(std::shared_ptr<VkResourceManager> newResourceManager,
                      RenderPassConfig newConfig);
  ~RenderPass() = default;

  RenderPass(const RenderPass&) = default;
  RenderPass(RenderPass&&) = delete;
  auto operator=(const RenderPass&) -> RenderPass& = default;
  auto operator=(RenderPass&&) -> RenderPass& = delete;

  [[nodiscard]] auto accepts(const RenderConfig& config) const -> bool;

  auto addDrawContext(RenderConfigHandle handle, DrawContext* drawContext) -> void;

  auto execute(const Frame* frame, vk::raii::CommandBuffer& cmdBuffer) -> void;

private:
  std::shared_ptr<VkResourceManager> resourceManager;
  RenderPassConfig config;
  std::unordered_map<RenderConfigHandle, DrawContext*> drawContexts;
};

}
