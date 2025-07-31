#include "ForwardGraphicsPass.hpp"
#include "img/ImageManager.hpp"
#include "r3/draw-context/ContextFactory.hpp"
#include "r3/draw-context/IDispatchContext.hpp"
#include "r3/graph/ImageUsageInfo.hpp"
#include "r3/graph/PassGraphInfo.hpp"
#include "r3/graph/ResourceAliasRegistry.hpp"
#include "r3/render-pass/PipelineCreateInfo.hpp"
#include "r3/render-pass/PipelineFactory.hpp"
#include "task/Frame.hpp"
#include "vk/sb/DSLayout.hpp"
#include "vk/sb/DSLayoutManager.hpp"

namespace tr {

const std::filesystem::path SHADER_ROOT = std::filesystem::current_path() / "assets" / "shaders";

ForwardGraphicsPass::ForwardGraphicsPass(std::shared_ptr<ImageManager> newImageManager,
                                         std::shared_ptr<ContextFactory> newDrawContextFactory,
                                         std::shared_ptr<ResourceAliasRegistry> newAliasRegistry,
                                         std::shared_ptr<PipelineFactory> newPipelineFactory,
                                         std::shared_ptr<DSLayoutManager> newLayoutManager,
                                         const ForwardPassCreateInfo& createInfo,
                                         PassId newPassId)
    : imageManager{std::move(newImageManager)},
      drawContextFactory{std::move(newDrawContextFactory)},
      aliasRegistry{std::move(newAliasRegistry)},
      pipelineFactory{std::move(newPipelineFactory)},
      layoutManager{std::move(newLayoutManager)},
      colorAlias{createInfo.colorImage},
      depthAlias{createInfo.depthImage},
      id{newPassId} {
  Log.trace("Creating ForwardGraphicsPass");

  const auto colorHandle = aliasRegistry->getHandle(colorAlias);
  const auto colorFormats = {imageManager->getImageMetadata(colorHandle).format};
  // HACK
  const auto extent = imageManager->getImageMetadata(colorHandle).extent;

  const auto depthHandle = aliasRegistry->getHandle(depthAlias);
  const auto depthFormat = imageManager->getImageMetadata(depthHandle).format;

  const auto vertexStage = ShaderStageInfo{
      .stage = vk::ShaderStageFlagBits::eVertex,
      .shaderFile = (SHADER_ROOT / "indirect.vert.spv").string(),
      .entryPoint = "main",
  };

  const auto fragmentStage = ShaderStageInfo{
      .stage = vk::ShaderStageFlagBits::eFragment,
      .shaderFile = (SHADER_ROOT / "indirect.frag.spv").string(),
      .entryPoint = "main",
  };

  auto dsLayoutList = std::vector<vk::DescriptorSetLayout>{};
  for (const auto& layoutHandle : createInfo.dsLayoutHandles) {
    dsLayoutList.push_back(layoutManager->getLayout(layoutHandle).getVkLayout());
  }

  const auto pipelineLayoutInfo = PipelineLayoutInfo{
      .pushConstantInfoList = {PushConstantInfo{.stageFlags = vk::ShaderStageFlagBits::eVertex |
                                                              vk::ShaderStageFlagBits::eFragment,
                                                .offset = 0,
                                                .size = 104}},
      .descriptorSetLayouts = dsLayoutList};

  const auto pipelineCreateInfo =
      PipelineCreateInfo{.id = id,
                         .pipelineType = PipelineType::Graphics,
                         .pipelineLayoutInfo = pipelineLayoutInfo,
                         .colorAttachmentFormats = colorFormats,
                         .depthAttachmentFormat = depthFormat,
                         .shaderStageInfo = {vertexStage, fragmentStage}};

  auto [layout, pipeline] = pipelineFactory->createPipeline(pipelineCreateInfo);

  this->pipelineLayout.emplace(std::move(layout));
  this->pipeline.emplace(std::move(pipeline));

  renderingInfo =
      vk::RenderingInfo{.renderArea = vk::Rect2D{.offset = {.x = 0, .y = 0}, .extent = extent},
                        .layerCount = 1};

  colorAttachmentInfo = std::make_optional(vk::RenderingAttachmentInfo{
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue =
          vk::ClearValue{
              .color = vk::ClearColorValue{std::array<float, 4>{0.392f, 0.584f, 0.929f, 1.0f}}},
  });

  depthAttachmentInfo = std::make_optional(vk::RenderingAttachmentInfo{
      .imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue =
          vk::ClearValue{.depthStencil = vk::ClearDepthStencilValue{.depth = 1.f, .stencil = 0}},
  });
}

[[nodiscard]] auto ForwardGraphicsPass::getId() const -> PassId {
  return id;
}

auto ForwardGraphicsPass::execute(Frame* frame, vk::raii::CommandBuffer& cmdBuffer) -> void {
  const auto handle = aliasRegistry->getHandle(colorAlias);
  const auto& image = imageManager->getImage(frame->getLogicalImage(handle));
  colorAttachmentInfo->setImageView(image.getImageView());

  const auto depthHandle = aliasRegistry->getHandle(depthAlias);
  const auto& depthImage = imageManager->getImage(frame->getLogicalImage(depthHandle));
  depthAttachmentInfo->setImageView(depthImage.getImageView());

  renderingInfo.setColorAttachmentCount(1);
  renderingInfo.setColorAttachments(*colorAttachmentInfo);
  renderingInfo.setPDepthAttachment(&*depthAttachmentInfo);

  if (!frame->getImageTransitionInfo().empty()) {
    auto barriers = std::vector<vk::ImageMemoryBarrier2>{};
    for (const auto& info : frame->getImageTransitionInfo()) {
      barriers.push_back({.srcStageMask = vk::PipelineStageFlagBits2::eAllCommands,
                          .srcAccessMask = vk::AccessFlagBits2::eNone,
                          .dstStageMask = vk::PipelineStageFlagBits2::eAllCommands,
                          .dstAccessMask = vk::AccessFlagBits2::eNone,
                          .oldLayout = info.oldLayout,
                          .newLayout = info.newLayout,
                          .image = info.image,
                          .subresourceRange = info.subresourceRange});
    }

    if (!barriers.empty()) {
      cmdBuffer.pipelineBarrier2({.imageMemoryBarrierCount = static_cast<uint32_t>(barriers.size()),
                                  .pImageMemoryBarriers = barriers.data()});
    }
  }

  cmdBuffer.beginRendering(renderingInfo);
  cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);

  for (const auto& handle : drawableContexts) {
    const auto& drawContext = drawContextFactory->getDispatchContext(handle);
    drawContext->bind(frame, cmdBuffer, *pipelineLayout);
    drawContext->dispatch(frame, cmdBuffer);
  }

  cmdBuffer.endRendering();
}

auto ForwardGraphicsPass::registerDispatchContext(Handle<IDispatchContext> handle) -> void {
  drawableContexts.push_back(handle);
}

[[nodiscard]] auto ForwardGraphicsPass::getGraphInfo() const -> PassGraphInfo {
  auto graphInfo = PassGraphInfo{};

  for (const auto& handle : drawableContexts) {
    const auto& dispatchContext = drawContextFactory->getDispatchContext(handle);
    const auto contextInfo = dispatchContext->getGraphInfo();
    graphInfo.bufferReads.insert(contextInfo.bufferReads.begin(), contextInfo.bufferReads.end());
    graphInfo.bufferWrites.insert(contextInfo.bufferWrites.begin(), contextInfo.bufferWrites.end());
    graphInfo.imageReads.insert(contextInfo.imageReads.begin(), contextInfo.imageReads.end());
    graphInfo.imageWrites.insert(contextInfo.imageWrites.begin(), contextInfo.imageWrites.end());
  }

  return graphInfo;
}

}
