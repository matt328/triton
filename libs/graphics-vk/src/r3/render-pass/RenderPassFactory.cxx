#include "RenderPassFactory.hpp"
#include "gfx/IFrameManager.hpp"
#include "img/ImageManager.hpp"
#include "r3/render-pass/ComputePass.hpp"
#include "r3/render-pass/ComputePassConfig.hpp"
#include "r3/render-pass/GraphicsPass.hpp"
#include "r3/render-pass/GraphicsPassConfig.hpp"
#include "r3/render-pass/PipelineFactory.hpp"
#include "task/Frame.hpp"
#include <ranges>

namespace tr {

RenderPassFactory::RenderPassFactory(std::shared_ptr<PipelineFactory> newPipelineFactory,
                                     std::shared_ptr<ImageManager> newImageManager,
                                     std::shared_ptr<IFrameManager> newFrameManager)
    : pipelineFactory{std::move(newPipelineFactory)},
      imageManager{std::move(newImageManager)},
      frameManager{std::move(newFrameManager)} {
}

auto RenderPassFactory::createGraphicsPass(const GraphicsPassCreateInfo& createInfo)
    -> std::unique_ptr<GraphicsPass> {
  Log.trace("RenderPassFactory::createGraphicsPass()");

  auto formats = createInfo.colorAttachmentInfos |
                 std::views::transform(&AttachmentCreateInfo::format) |
                 std::ranges::to<std::vector<vk::Format>>();

  const auto pipelineCreateInfo = PipelineCreateInfo{
      .id = createInfo.id,
      .pipelineType = PipelineType::Graphics,
      .pipelineLayoutInfo = createInfo.pipelineLayoutInfo,
      .colorAttachmentFormats = formats,
      .depthAttachmentFormat = createInfo.depthAttachmentFormat.has_value()
                                   ? std::optional{createInfo.depthAttachmentFormat->format}
                                   : std::nullopt,
      .shaderStageInfo = createInfo.shaderStageInfo};
  auto [layout, pipeline] = pipelineFactory->createPipeline(pipelineCreateInfo);

  auto colorAttachmentList = std::vector<AttachmentConfig>{};
  colorAttachmentList.reserve(createInfo.colorAttachmentInfos.size());
  for (const auto& info : createInfo.colorAttachmentInfos) {
    const auto request = ImageRequest{
        .logicalName = std::format("{}", createInfo.id),
        .format = info.format,
        .extent = createInfo.extent,
        .usageFlags = vk::ImageUsageFlagBits::eColorAttachment,
        .aspectFlags = vk::ImageAspectFlagBits::eColor,
    };
    const auto logicalHandle = imageHandleGenerator.requestLogicalHandle();
    for (const auto& frame : frameManager->getFrames()) {
      const auto handle = imageManager->createImage(request);
      frame->addLogicalImage(logicalHandle, handle);
    }
    colorAttachmentList.push_back({
        .logicalImage = logicalHandle,
        .clearValue = info.clearValue,
    });
  }

  auto config = GraphicsPassConfig{
      .id = createInfo.id,
      .pipeline = std::move(pipeline),
      .pipelineLayout = std::move(layout),
      .colorAttachmentConfigs = colorAttachmentList,
      .extent = createInfo.extent,
  };

  return std::make_unique<GraphicsPass>(std::move(config), imageManager);
}

auto RenderPassFactory::createComputePass(const ComputePassCreateInfo& createInfo)
    -> std::unique_ptr<ComputePass> {
  const auto pipelineCreateInfo =
      PipelineCreateInfo{.id = createInfo.id,
                         .pipelineType = PipelineType::Compute,
                         .pipelineLayoutInfo = createInfo.pipelineLayoutInfo,
                         .shaderStageInfo = {createInfo.shaderStageInfo}};
  auto [layout, pipeline] = pipelineFactory->createPipeline(pipelineCreateInfo);

  auto config = ComputePassConfig{
      .id = createInfo.id,
      .pipeline = std::move(pipeline),
      .pipelineLayout = std::move(layout),
      .debugName = createInfo.id,
  };

  return std::make_unique<ComputePass>(std::move(config), imageManager);
}

}
