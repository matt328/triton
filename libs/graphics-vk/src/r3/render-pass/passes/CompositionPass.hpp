#pragma once

#include "r3/render-pass/CreateInfo.hpp"
#include "r3/render-pass/IRenderPass.hpp"

namespace tr {

class ContextFactory;
class ImageManager;
class ResourceAliasRegistry;
class PipelineFactory;

class CompositionPass : public IRenderPass {
public:
  CompositionPass(std::shared_ptr<ImageManager> newImageManager,
                  std::shared_ptr<ContextFactory> newDrawContextFactory,
                  std::shared_ptr<ResourceAliasRegistry> newAliasRegistry,
                  std::shared_ptr<PipelineFactory> newPipelineFactory,
                  CompositionPassCreateInfo createInfo,
                  PassId newPassId);
  ~CompositionPass() = default;

  CompositionPass(const CompositionPass&) = delete;
  CompositionPass(CompositionPass&&) = delete;
  auto operator=(const CompositionPass&) -> CompositionPass& = delete;
  auto operator=(CompositionPass&&) -> CompositionPass& = delete;

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
  ImageAlias swapchainAlias;
  PassId id;

  std::optional<vk::raii::Pipeline> pipeline;
  std::optional<vk::raii::PipelineLayout> pipelineLayout;

  vk::RenderingInfo renderingInfo;
  std::optional<vk::RenderingAttachmentInfo> colorAttachmentInfo;

  std::vector<Handle<IDispatchContext>> drawableContexts;
};

}
