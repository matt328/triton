#include "r3/graph/BarrierGenerator.hpp"

#include "MockRenderPass.hpp"

namespace tr {

TEST_CASE("BarrierGenerator generates correct image and buffer barriers") {
  BarrierGenerator generator;

  // Pass A: writes GeometryColorImage, writes IndirectCommand
  auto passA = std::make_unique<MockRenderPass>(
      PassId::Culling,
      PassGraphInfo{
          .imageWrites = {ImageUsageInfo{
              .alias = ImageAlias::GeometryColorImage,
              .accessFlags = vk::AccessFlagBits2::eColorAttachmentWrite,
              .stageFlags = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
              .aspectFlags = vk::ImageAspectFlagBits::eColor,
              .layout = vk::ImageLayout::eColorAttachmentOptimal,
          }},
          .imageReads = {ImageUsageInfo{
              .alias = ImageAlias::DepthImage,
              .accessFlags = vk::AccessFlagBits2::eShaderRead,
              .stageFlags = vk::PipelineStageFlagBits2::eComputeShader,
              .aspectFlags = vk::ImageAspectFlagBits::eColor,
              .layout = vk::ImageLayout::eShaderReadOnlyOptimal,
          }},
          .bufferWrites = {{
              .alias = BufferAlias::IndirectCommand,
              .accessFlags = vk::AccessFlagBits2::eVertexAttributeRead,
              .stageFlags = vk::PipelineStageFlagBits2::eVertexInput,
          }},
      });

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
          }},
          .imageReads = {ImageUsageInfo{
              .alias = ImageAlias::GeometryColorImage,
              .accessFlags = vk::AccessFlagBits2::eShaderRead,
              .stageFlags = vk::PipelineStageFlagBits2::eFragmentShader,
              .aspectFlags = vk::ImageAspectFlagBits::eColor,
              .layout = vk::ImageLayout::eShaderReadOnlyOptimal,
          }},
          .bufferReads = {{
              .alias = BufferAlias::IndirectCommand,
              .accessFlags = vk::AccessFlagBits2::eVertexAttributeRead,
              .stageFlags = vk::PipelineStageFlagBits2::eVertexInput,
          }},
      });

  std::vector<std::unique_ptr<IRenderPass>> passes;
  passes.push_back(std::move(passA));
  passes.push_back(std::move(passB));

  auto plan = generator.build(passes);

  SECTION("Forward pass has image barrier from Culling") {
    auto& imageBarriers = plan.imageBarriers[PassId::Forward];
    REQUIRE(imageBarriers.size() == 2);
    const auto& barrier = imageBarriers[0];
    REQUIRE(barrier.alias == ImageAlias::GeometryColorImage);
    REQUIRE(barrier.imageBarrier.srcAccessMask == vk::AccessFlagBits2::eColorAttachmentWrite);
    REQUIRE(barrier.imageBarrier.dstAccessMask == vk::AccessFlagBits2::eShaderRead);
    REQUIRE(barrier.imageBarrier.oldLayout == vk::ImageLayout::eColorAttachmentOptimal);
    REQUIRE(barrier.imageBarrier.newLayout == vk::ImageLayout::eShaderReadOnlyOptimal);
  }

  SECTION("Forward pass has buffer barrier from Culling") {
    auto& bufferBarriers = plan.bufferBarriers[PassId::Forward];
    REQUIRE(bufferBarriers.size() == 1);
    const auto& barrier = bufferBarriers[0];
    REQUIRE(barrier.alias == BufferAlias::IndirectCommand);
    REQUIRE(barrier.bufferBarrier.srcAccessMask == vk::AccessFlagBits2::eVertexAttributeRead);
    REQUIRE(barrier.bufferBarrier.dstAccessMask == vk::AccessFlagBits2::eVertexAttributeRead);
  }
}
}
