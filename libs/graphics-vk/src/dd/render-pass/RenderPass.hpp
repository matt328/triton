#pragma once

#include "dd/DrawContext.hpp"
#include "dd/RenderConfigHandle.hpp"
#include "dd/render-pass/RenderPassConfig.hpp"

namespace tr {

class DrawContextFactory;
class Frame;
class ImageManager;

class RenderPass {
public:
  explicit RenderPass(RenderPassConfig&& newConfig, std::shared_ptr<ImageManager> newImageManager);
  ~RenderPass() = default;

  RenderPass(const RenderPass&) = delete;
  RenderPass(RenderPass&&) = delete;
  auto operator=(const RenderPass&) -> RenderPass& = delete;
  auto operator=(RenderPass&&) -> RenderPass& = delete;

  [[nodiscard]] auto accepts(const RenderConfig& config) const -> bool;

  auto addDrawContext(RenderConfigHandle handle, DrawContext* drawContext) -> void;

  /// RenderPass will bind its pipeline and push constants
  /// After this, the Renderer will have an opportunity to bind any global DescriptorSets
  auto bind(const Frame* frame, vk::raii::CommandBuffer& cmdBuffer) -> void;

  auto draw(const Frame* frame, vk::raii::CommandBuffer& cmdBuffer) -> void;

private:
  std::shared_ptr<ImageManager> imageManager;
  RenderPassConfig config;
  std::unordered_map<RenderConfigHandle, DrawContext*> drawContexts;
};

}
