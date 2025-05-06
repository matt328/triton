#pragma once

#include "r3/draw-context/DrawContext.hpp"
#include "r3/render-pass/GraphicsPassConfig.hpp"
#include "r3/render-pass/BindFunctions.hpp"

namespace tr {

class ImageManager;
class Frame;
class ContextFactory;

class GraphicsPass {
public:
  GraphicsPass(GraphicsPassConfig&& config,
               std::shared_ptr<ImageManager> newImageManager,
               std::shared_ptr<ContextFactory> newDrawContextFactory);
  ~GraphicsPass() = default;

  GraphicsPass(const GraphicsPass&) = delete;
  GraphicsPass(GraphicsPass&&) = delete;
  auto operator=(const GraphicsPass&) -> GraphicsPass& = delete;
  auto operator=(GraphicsPass&&) -> GraphicsPass& = delete;

  auto execute(const Frame* frame, vk::raii::CommandBuffer& cmdBuffer) -> void;

  auto registerDrawContext(Handle<DrawContext> handle) -> void;

  [[nodiscard]] auto getId() const {
    return passConfig.id;
  }

private:
  std::shared_ptr<ImageManager> imageManager;
  std::shared_ptr<ContextFactory> drawContextFactory;
  GraphicsPassConfig passConfig;

  vk::RenderingInfo renderingInfo;
  std::vector<vk::RenderingAttachmentInfo> colorAttachmentInfo;
  std::optional<vk::RenderingAttachmentInfo> depthAttachmentInfo;

  std::vector<Handle<DrawContext>> drawableContexts;
};

}
