#pragma once

#include "IRenderPass.hpp"
#include "r3/ComponentIds.hpp"
#include "r3/graph/ResourceAliases.hpp"
#include "r3/render-pass/CreateInfo.hpp"

namespace tr {

class ImageManager;
class ContextFactory;
class ResourceAliasRegistry;
class PipelineFactory;

struct ImageUse {
  ImageAlias color;
  ImageAlias depth;
};

class ForwardGraphicsPass : public IRenderPass {
public:
  ForwardGraphicsPass(std::shared_ptr<ImageManager> newImageManager,
                      std::shared_ptr<ContextFactory> newDrawContextFactory,
                      std::shared_ptr<ResourceAliasRegistry> newAliasRegistry,
                      std::shared_ptr<PipelineFactory> newPipelineFactory,
                      ForwardPassCreateInfo createInfo,
                      PassId newPassId);
  ~ForwardGraphicsPass() override = default;

  ForwardGraphicsPass(const ForwardGraphicsPass&) = delete;
  ForwardGraphicsPass(ForwardGraphicsPass&&) = delete;
  auto operator=(const ForwardGraphicsPass&) -> ForwardGraphicsPass& = delete;
  auto operator=(ForwardGraphicsPass&&) -> ForwardGraphicsPass& = delete;

  [[nodiscard]] auto getId() const -> PassId override;
  auto execute(const Frame* frame, vk::raii::CommandBuffer& cmdBuffer) -> void override;
  auto registerDispatchContext(Handle<IDispatchContext> handle) -> void override;
  [[nodiscard]] auto getGraphInfo() const -> PassGraphInfo override;

private:
  std::shared_ptr<ImageManager> imageManager;
  std::shared_ptr<ContextFactory> drawContextFactory;
  std::shared_ptr<ResourceAliasRegistry> aliasRegistry;
  std::shared_ptr<PipelineFactory> pipelineFactory;
  ImageAlias colorAlias;
  ImageAlias depthAlias;
  PassId id;

  std::optional<vk::raii::Pipeline> pipeline;
  std::optional<vk::raii::PipelineLayout> pipelineLayout;

  vk::RenderingInfo renderingInfo;
  std::optional<vk::RenderingAttachmentInfo> colorAttachmentInfo;
  std::optional<vk::RenderingAttachmentInfo> depthAttachmentInfo;

  std::vector<Handle<IDispatchContext>> drawableContexts;
};

}
