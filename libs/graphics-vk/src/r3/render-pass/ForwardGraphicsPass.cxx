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

namespace tr {

const std::filesystem::path SHADER_ROOT = std::filesystem::current_path() / "assets" / "shaders";

ForwardGraphicsPass::ForwardGraphicsPass(std::shared_ptr<ImageManager> newImageManager,
                                         std::shared_ptr<ContextFactory> newDrawContextFactory,
                                         std::shared_ptr<ResourceAliasRegistry> newAliasRegistry,
                                         std::shared_ptr<PipelineFactory> newPipelineFactory,
                                         ForwardPassCreateInfo createInfo,
                                         PassId newPassId)
    : imageManager{std::move(newImageManager)},
      drawContextFactory{std::move(newDrawContextFactory)},
      aliasRegistry{std::move(newAliasRegistry)},
      pipelineFactory{std::move(newPipelineFactory)},
      colorAlias{createInfo.colorImage},
      depthAlias{createInfo.depthImage},
      id{newPassId} {
  Log.trace("Creating ForwardGraphicsPass");

  const auto colorHandle = aliasRegistry->getHandle(colorAlias);
  const auto colorFormats = {imageManager->getImageMetadata(colorHandle).format};
  // HACK
  const auto extent = imageManager->getImageMetadata(colorHandle).extent;

  const auto depthHandle = newAliasRegistry->getHandle(depthAlias);
  const auto depthFormat = imageManager->getImageMetadata(depthHandle).format;

  const auto vertexStage = ShaderStageInfo{
      .stage = vk::ShaderStageFlagBits::eVertex,
      .shaderFile = (SHADER_ROOT / "static.vert.spv").string(),
      .entryPoint = "main",
  };

  const auto fragmentStage = ShaderStageInfo{
      .stage = vk::ShaderStageFlagBits::eFragment,
      .shaderFile = (SHADER_ROOT / "static.frag.spv").string(),
      .entryPoint = "main",
  };

  const auto pipelineLayoutInfo = PipelineLayoutInfo{
      .pushConstantInfoList = {PushConstantInfo{.stageFlags = vk::ShaderStageFlagBits::eVertex,
                                                .offset = 0,
                                                .size = 36}}};

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

  // Figure out what is needed for the passgraph info so that the framegraph can take a previous
  // pass outputs and a next pass inputs and create barriers and transitions automatically

  // Create a PassGraphInfo that describes the ResourceAliases and how they are used so the frame
  // graph can transition them
}

[[nodiscard]] auto ForwardGraphicsPass::getId() const -> PassId {
  return id;
}

auto ForwardGraphicsPass::execute(const Frame* frame, vk::raii::CommandBuffer& cmdBuffer) -> void {
  const auto handle = aliasRegistry->getHandle(colorAlias);
  const auto& image = imageManager->getImage(frame->getLogicalImage(handle));
  colorAttachmentInfo->setImageView(image.getImageView());

  const auto depthHandle = aliasRegistry->getHandle(depthAlias);
  const auto& depthImage = imageManager->getImage(frame->getLogicalImage(depthHandle));
  depthAttachmentInfo->setImageView(depthImage.getImageView());

  renderingInfo.setColorAttachmentCount(1);
  renderingInfo.setColorAttachments(*colorAttachmentInfo);
  renderingInfo.setPDepthAttachment(&*depthAttachmentInfo);

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
  return PassGraphInfo{
      .imageWrites = {
          ImageUsageInfo{
              .alias = colorAlias,
              .accessFlags = vk::AccessFlagBits2::eColorAttachmentWrite,
              .stageFlags = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
              .aspectFlags = vk::ImageAspectFlagBits::eColor,
              .layout = vk::ImageLayout::eColorAttachmentOptimal,
              .clearValue = vk::ClearValue{.color = {std::array<float, 4>{0.f, 0.f, 0.f, 1.f}}},
          },
          ImageUsageInfo{
              .alias = depthAlias,
              .accessFlags = vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
              .stageFlags = vk::PipelineStageFlagBits2::eEarlyFragmentTests,
              .aspectFlags = vk::ImageAspectFlagBits::eDepth,
              .layout = vk::ImageLayout::eDepthAttachmentOptimal,
              .clearValue =
                  vk::ClearValue{.depthStencil =
                                     vk::ClearDepthStencilValue{.depth = 1.0f, .stencil = 0}},
          }}};
}

}
