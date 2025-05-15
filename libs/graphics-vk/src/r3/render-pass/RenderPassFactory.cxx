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
                                     std::shared_ptr<IFrameManager> newFrameManager,
                                     std::shared_ptr<ContextFactory> newDrawContextFactory)
    : pipelineFactory{std::move(newPipelineFactory)},
      imageManager{std::move(newImageManager)},
      frameManager{std::move(newFrameManager)},
      drawContextFactory{std::move(newDrawContextFactory)} {
}

auto RenderPassFactory::createGraphicsPass(const GraphicsPassCreateInfo& createInfo)
    -> std::unique_ptr<GraphicsPass> {
  Log.trace("RenderPassFactory::createGraphicsPass()");

  auto colorFormats = std::vector<vk::Format>{};
  std::optional<vk::Format> depthFormat = std::nullopt;
  for (const auto& imageUsage : createInfo.outputs) {
    if (imageUsage.accessFlags & vk::AccessFlagBits::eColorAttachmentWrite) {
      colorFormats.push_back(imageUsage.imageFormat);
    }
    if (imageUsage.accessFlags & vk::AccessFlagBits::eDepthStencilAttachmentWrite) {
      depthFormat = imageUsage.imageFormat;
    }
  }

  const auto pipelineCreateInfo =
      PipelineCreateInfo{.id = createInfo.id,
                         .pipelineType = PipelineType::Graphics,
                         .pipelineLayoutInfo = createInfo.pipelineLayoutInfo,
                         .colorAttachmentFormats = colorFormats,
                         .depthAttachmentFormat = depthFormat,
                         .shaderStageInfo = createInfo.shaderStageInfo};
  auto [layout, pipeline] = pipelineFactory->createPipeline(pipelineCreateInfo);

  // Extract color and depth attachment info
  auto colorAttachmentList = std::vector<AttachmentConfig>{};
  std::optional<AttachmentConfig> depthAttachmentConfig = std::nullopt;
  for (const auto& imageUsageInfo : createInfo.outputs) {
    if (imageUsageInfo.accessFlags & vk::AccessFlagBits::eColorAttachmentWrite) {
      colorAttachmentList.push_back(
          {.logicalImage = imageUsageInfo.imageHandle, .clearValue = imageUsageInfo.clearValue});
    }
    if (imageUsageInfo.accessFlags & vk::AccessFlagBits::eDepthStencilAttachmentWrite) {
      depthAttachmentConfig = AttachmentConfig{.logicalImage = imageUsageInfo.imageHandle,
                                               .clearValue = imageUsageInfo.clearValue};
    }
  }

  auto config = GraphicsPassConfig{
      .id = createInfo.id,
      .pipeline = std::move(pipeline),
      .pipelineLayout = std::move(layout),
      .colorAttachmentConfigs = colorAttachmentList,
      .depthAttachmentConfig = depthAttachmentConfig,
      .extent = createInfo.extent,
  };

  return std::make_unique<GraphicsPass>(std::move(config), imageManager, drawContextFactory);
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

  return std::make_unique<ComputePass>(std::move(config), imageManager, drawContextFactory);
}

}
