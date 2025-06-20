#include "BarrierGenerator.hpp"
#include "r3/render-pass/IRenderPass.hpp"

namespace tr {

// NOTE: Barrier before use not after production

auto BarrierGenerator::build(const std::vector<std::unique_ptr<IRenderPass>>& passes)
    -> BarrierPlan {
  auto barrierPlan = BarrierPlan{};
  for (const auto& pass : passes) {
    const auto currentPassId = pass->getId();
    const auto& info = pass->getGraphInfo();

    // Image Reads
    for (const auto& read : info.imageReads) {
      const auto alias = read.alias;
      if (lastImageUses.contains(alias)) {
        const auto& last = lastImageUses[alias];
        barrierPlan.imageBarriers[currentPassId].push_back(ImageBarrierData{
            .imageBarrier = vk::ImageMemoryBarrier2{.srcStageMask = last.stageMask,
                                                    .srcAccessMask = last.accessMask,
                                                    .dstStageMask = read.stageFlags,
                                                    .dstAccessMask = read.accessFlags,
                                                    .oldLayout = last.layout,
                                                    .newLayout = read.layout,
                                                    .subresourceRange =
                                                        vk::ImageSubresourceRange{
                                                            .aspectMask = read.aspectFlags,
                                                            .baseMipLevel = 0,
                                                            .levelCount = 1,
                                                            .baseArrayLayer = 0,
                                                            .layerCount = 1,
                                                        }},
            .alias = alias,
        });
      }
      lastImageUses[alias] = LastImageUse{.passId = currentPassId,
                                          .accessMask = read.accessFlags,
                                          .stageMask = read.stageFlags,
                                          .layout = read.layout};
    }

    // Image Writes
    for (const auto& write : info.imageWrites) {
      const auto alias = write.alias;
      if (lastImageUses.contains(alias)) {
        const auto& last = lastImageUses[alias];
        barrierPlan.imageBarriers[currentPassId].push_back(ImageBarrierData{
            .imageBarrier = vk::ImageMemoryBarrier2{.srcStageMask = last.stageMask,
                                                    .srcAccessMask = last.accessMask,
                                                    .dstStageMask = write.stageFlags,
                                                    .dstAccessMask = write.accessFlags,
                                                    .oldLayout = last.layout,
                                                    .newLayout = write.layout,
                                                    .subresourceRange =
                                                        vk::ImageSubresourceRange{
                                                            .aspectMask = write.aspectFlags,
                                                            .baseMipLevel = 0,
                                                            .levelCount = 1,
                                                            .baseArrayLayer = 0,
                                                            .layerCount = 1,
                                                        }},
            .alias = alias,
        });
      } else if (alias == ImageAlias::SwapchainImage) {
        barrierPlan.imageBarriers[currentPassId].push_back(ImageBarrierData{
            .imageBarrier =
                vk::ImageMemoryBarrier2{.srcStageMask = {},
                                        .srcAccessMask = {},
                                        .dstStageMask = write.stageFlags,
                                        .dstAccessMask = write.accessFlags,
                                        .oldLayout =
                                            vk::ImageLayout::eUndefined, // or ePresentSrcKHR
                                        .newLayout = write.layout,
                                        .subresourceRange =
                                            vk::ImageSubresourceRange{
                                                .aspectMask = write.aspectFlags,
                                                .baseMipLevel = 0,
                                                .levelCount = 1,
                                                .baseArrayLayer = 0,
                                                .layerCount = 1,
                                            }},
            .alias = alias,
        });
      }
      lastImageUses[alias] = LastImageUse{.passId = currentPassId,
                                          .accessMask = write.accessFlags,
                                          .stageMask = write.stageFlags,
                                          .layout = write.layout};
    }

    // Buffer Reads
    for (const auto& read : info.bufferReads) {
      const auto alias = read.alias;
      if (lastBufferUses.contains(alias)) {
        const auto& last = lastBufferUses[alias];
        barrierPlan.bufferBarriers[currentPassId].push_back(BufferBarrierData{
            .bufferBarrier = vk::BufferMemoryBarrier2{.srcStageMask = last.stageMask,
                                                      .srcAccessMask = last.accessMask,
                                                      .dstStageMask = read.stageFlags,
                                                      .dstAccessMask = read.accessFlags,
                                                      .size = vk::WholeSize},
            .alias = alias,
        });
      }
      lastBufferUses[alias] = LastBufferUse{.passId = currentPassId,
                                            .accessMask = read.accessFlags,
                                            .stageMask = read.stageFlags};
    }

    // Buffer Reads
    for (const auto& write : info.bufferWrites) {
      const auto alias = write.alias;
      if (lastBufferUses.contains(alias)) {
        const auto& last = lastBufferUses[alias];
        barrierPlan.bufferBarriers[currentPassId].push_back(BufferBarrierData{
            .bufferBarrier = vk::BufferMemoryBarrier2{.srcStageMask = last.stageMask,
                                                      .srcAccessMask = last.accessMask,
                                                      .dstStageMask = write.stageFlags,
                                                      .dstAccessMask = write.accessFlags,
                                                      .size = vk::WholeSize},
            .alias = alias,
        });
      }
      lastBufferUses[alias] = LastBufferUse{.passId = currentPassId,
                                            .accessMask = write.accessFlags,
                                            .stageMask = write.stageFlags};
    }
  }

  barrierPlan.swapchainBarrier = ImageBarrierData{
      .imageBarrier =
          vk::ImageMemoryBarrier2{
              .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
              .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
              .dstStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe,
              .dstAccessMask = vk::AccessFlagBits2::eNone,
              .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
              .newLayout = vk::ImageLayout::ePresentSrcKHR,
              .subresourceRange =
                  vk::ImageSubresourceRange{
                      .aspectMask = vk::ImageAspectFlagBits::eColor,
                      .baseMipLevel = 0,
                      .levelCount = 1,
                      .baseArrayLayer = 0,
                      .layerCount = 1,
                  },
          },
      .alias = ImageAlias::SwapchainImage,
  };

  return barrierPlan;
}

}
