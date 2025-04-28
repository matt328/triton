#pragma once

#include "r3/render-pass/GraphicsPassCreateInfo.hpp"

namespace tr {

class GraphicsPass;
class PipelineFactory;
class ImageManager;

class RenderPassFactory {
public:
  RenderPassFactory(std::shared_ptr<PipelineFactory> newPipelineFactory,
                    std::shared_ptr<ImageManager> newImageManager);
  ~RenderPassFactory() = default;

  RenderPassFactory(const RenderPassFactory&) = default;
  RenderPassFactory(RenderPassFactory&&) = delete;
  auto operator=(const RenderPassFactory&) -> RenderPassFactory& = default;
  auto operator=(RenderPassFactory&&) -> RenderPassFactory& = delete;

  auto createGraphicsPass(const GraphicsPassCreateInfo& createInfo)
      -> std::unique_ptr<GraphicsPass>;

private:
  std::shared_ptr<PipelineFactory> pipelineFactory;
  std::shared_ptr<ImageManager> imageManager;
};

}
