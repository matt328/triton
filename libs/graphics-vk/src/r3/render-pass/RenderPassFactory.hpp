#pragma once

#include "bk/HandleGenerator.hpp"
#include "r3/render-pass/GraphicsPassCreateInfo.hpp"
#include "r3/render-pass/ComputePassCreateInfo.hpp"

namespace tr {

class GraphicsPass;
class ComputePass;
class PipelineFactory;
class ImageManager;
class IFrameManager;
class ManagedImage;

class RenderPassFactory {
public:
  RenderPassFactory(std::shared_ptr<PipelineFactory> newPipelineFactory,
                    std::shared_ptr<ImageManager> newImageManager,
                    std::shared_ptr<IFrameManager> newFrameManager);
  ~RenderPassFactory() = default;

  RenderPassFactory(const RenderPassFactory&) = delete;
  RenderPassFactory(RenderPassFactory&&) = delete;
  auto operator=(const RenderPassFactory&) -> RenderPassFactory& = delete;
  auto operator=(RenderPassFactory&&) -> RenderPassFactory& = delete;

  auto createGraphicsPass(const GraphicsPassCreateInfo& createInfo)
      -> std::unique_ptr<GraphicsPass>;

  auto createComputePass(const ComputePassCreateInfo& createInfo) -> std::unique_ptr<ComputePass>;

private:
  std::shared_ptr<PipelineFactory> pipelineFactory;
  std::shared_ptr<ImageManager> imageManager;
  std::shared_ptr<IFrameManager> frameManager;

  HandleGenerator<ManagedImage> imageHandleGenerator;
};

}
