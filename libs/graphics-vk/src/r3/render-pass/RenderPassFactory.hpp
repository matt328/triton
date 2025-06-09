#pragma once

#include "bk/HandleGenerator.hpp"
#include "r3/ComponentIds.hpp"
#include "r3/render-pass/ComputePassCreateInfo.hpp"

namespace tr {

class GraphicsPass;
class ComputePass;
class PipelineFactory;
class ImageManager;
class IFrameManager;
class ManagedImage;
class ContextFactory;
class IRenderPass;
class ResourceAliasRegistry;

enum class RenderPassType : uint8_t {
  Forward = 0,
  Composition,
  Culling,
  Count
};

class RenderPassFactory {
public:
  RenderPassFactory(std::shared_ptr<PipelineFactory> newPipelineFactory,
                    std::shared_ptr<ImageManager> newImageManager,
                    std::shared_ptr<IFrameManager> newFrameManager,
                    std::shared_ptr<ContextFactory> newDrawContextFactory,
                    std::shared_ptr<ResourceAliasRegistry> newAliasRegistry);
  ~RenderPassFactory() = default;

  RenderPassFactory(const RenderPassFactory&) = delete;
  RenderPassFactory(RenderPassFactory&&) = delete;
  auto operator=(const RenderPassFactory&) -> RenderPassFactory& = delete;
  auto operator=(RenderPassFactory&&) -> RenderPassFactory& = delete;

  auto createRenderPass(RenderPassType passType, PassId passId) -> std::unique_ptr<IRenderPass>;

private:
  std::shared_ptr<PipelineFactory> pipelineFactory;
  std::shared_ptr<ImageManager> imageManager;
  std::shared_ptr<IFrameManager> frameManager;
  std::shared_ptr<ContextFactory> drawContextFactory;
  std::shared_ptr<ResourceAliasRegistry> aliasRegistry;

  HandleGenerator<ManagedImage> imageHandleGenerator;
};

}
