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
                                                    .newLayout = read.layout},
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
                                                    .newLayout = write.layout},
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
                                                      .dstAccessMask = read.accessFlags},
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
                                                      .dstAccessMask = write.accessFlags},
            .alias = alias,
        });
      }
      lastBufferUses[alias] = LastBufferUse{.passId = currentPassId,
                                            .accessMask = write.accessFlags,
                                            .stageMask = write.stageFlags};
    }
  }
  return barrierPlan;
}

}
