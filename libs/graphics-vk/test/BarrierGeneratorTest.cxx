#include "MockRenderPass.hpp"

namespace tr {

TEST_CASE("BarrierGenerator generates correct image and buffer barriers") {

  // Pass A: writes GeometryColorImage, writes IndirectCommand
  auto passA = std::make_unique<MockRenderPass>(
      PassId::Culling,
      PassGraphInfo{.bufferWrites =
                        {
                            BufferUsageInfo{
                                .alias = BufferAlias::IndirectCommand,
                                .accessFlags = vk::AccessFlagBits2::eShaderWrite,
                                .stageFlags = vk::PipelineStageFlagBits2::eComputeShader,
                            },
                        },
                    .bufferReads = {BufferUsageInfo{
                        .alias = BufferAlias::ObjectData,
                        .accessFlags = vk::AccessFlagBits2::eShaderRead,
                        .stageFlags = vk::PipelineStageFlagBits2::eComputeShader,
                    }}});

  // Pass B: reads GeometryColorImage, reads IndirectCommand
  auto passB = std::make_unique<MockRenderPass>(
      PassId::Forward,
      PassGraphInfo{
          .imageWrites = {ImageUsageInfo{
                              .alias = ImageAlias::DepthImage,
                              .accessFlags = vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
                              .stageFlags = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                              .aspectFlags = vk::ImageAspectFlagBits::eColor,
                              .layout = vk::ImageLayout::eColorAttachmentOptimal,
                          },
                          ImageUsageInfo{
                              .alias = ImageAlias::GeometryColorImage,
                              .accessFlags = vk::AccessFlagBits2::eColorAttachmentWrite,
                              .stageFlags = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
                              .aspectFlags = vk::ImageAspectFlagBits::eColor,
                              .layout = vk::ImageLayout::eColorAttachmentOptimal,
                          }},
          .bufferReads = {{
              .alias = BufferAlias::IndirectCommand,
              .accessFlags = vk::AccessFlagBits2::eIndirectCommandRead,
              .stageFlags = vk::PipelineStageFlagBits2::eDrawIndirect,
          }},
      });
  // Pass C: reads GeometryColorImage, reads IndirectCommand
  auto passC = std::make_unique<MockRenderPass>(
      PassId::Composition,
      PassGraphInfo{
          .imageWrites = {ImageUsageInfo{
              .alias = ImageAlias::SwapchainImage,
              .accessFlags = vk::AccessFlagBits2::eColorAttachmentWrite,
              .stageFlags = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
              .aspectFlags = vk::ImageAspectFlagBits::eColor,
              .layout = vk::ImageLayout::eColorAttachmentOptimal,
          }},
          .imageReads = {ImageUsageInfo{
              .alias = ImageAlias::GeometryColorImage,
              .accessFlags = vk::AccessFlagBits2::eShaderSampledRead,
              .stageFlags = vk::PipelineStageFlagBits2::eFragmentShader,
              .aspectFlags = vk::ImageAspectFlagBits::eColor,
              .layout = vk::ImageLayout::eShaderReadOnlyOptimal,
          }},
      });

  std::vector<std::unique_ptr<IRenderPass>> passes;
  passes.push_back(std::move(passA));
  passes.push_back(std::move(passB));

  SECTION("Forward pass has a buffer barrier for IndirectCommand") {
  }

  SECTION("Forward Pass transitions its color image to color attachment") {
  }
}
}
