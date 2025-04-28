#include "RenderPassFactory.hpp"
#include "r3/render-pass/GraphicsPass.hpp"
#include "r3/render-pass/GraphicsPassConfig.hpp"
#include "r3/render-pass/PipelineFactory.hpp"

namespace tr {

RenderPassFactory::RenderPassFactory(std::shared_ptr<PipelineFactory> newPipelineFactory,
                                     std::shared_ptr<ImageManager> newImageManager)
    : pipelineFactory{std::move(newPipelineFactory)}, imageManager{std::move(newImageManager)} {
}

auto RenderPassFactory::createGraphicsPass(const GraphicsPassCreateInfo& createInfo)
    -> std::unique_ptr<GraphicsPass> {
  Log.trace("RenderPassFactory::createGraphicsPass()");

  const auto pipelineCreateInfo =
      PipelineCreateInfo{.id = createInfo.id,
                         .pipelineType = PipelineType::Graphics,
                         .pipelineLayoutInfo = createInfo.pipelineLayoutInfo,
                         .colorAttachmentFormats = createInfo.colorAttachmentFormats,
                         .depthAttachmentFormat = createInfo.depthAttachmentFormat,
                         .shaderStageInfo = createInfo.shaderStageInfo};

  auto [layout, pipeline] = pipelineFactory->createPipeline(pipelineCreateInfo);

  for (const auto& format : createInfo.colorAttachmentFormats) {
    // create an ImageRequest
    // get a logical handle from the FrameManager
    // use that to create an AttachmentConfig, add to the colorAttachments list
  }

  auto config = GraphicsPassConfig{.id = 4,
                                   .pipeline = std::move(pipeline),
                                   .pipelineLayout = std::move(layout),
                                   .extent = createInfo.extent};

  return std::make_unique<GraphicsPass>(std::move(config), imageManager);
}

}
