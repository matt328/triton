#include "BarrierBuilder.hpp"

namespace tr {

auto BarrierBuilder::build(const BufferBarrierPrecursor& bbp, std::optional<LastBufferUse> lastUse)
    -> std::optional<vk::BufferMemoryBarrier2> {
  const auto srcStage = lastUse ? lastUse->stageMask : vk::PipelineStageFlagBits2::eTopOfPipe;
  const auto srcAccess = lastUse ? lastUse->accessMask : vk::AccessFlagBits2::eNone;

  if (srcStage == bbp.stageFlags && srcAccess == bbp.accessFlags) {
    return std::nullopt;
  }

  return vk::BufferMemoryBarrier2{
      .srcStageMask = srcStage,
      .srcAccessMask = srcAccess,
      .dstStageMask = bbp.stageFlags,
      .dstAccessMask = bbp.accessFlags,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .offset = 0,
      .size = VK_WHOLE_SIZE,
  };
}

auto BarrierBuilder::build(const ImageBarrierPrecursor& ibp, std::optional<LastImageUse> lastUse)
    -> std::optional<vk::ImageMemoryBarrier2> {
  const auto oldLayout = lastUse ? lastUse->layout : vk::ImageLayout::eUndefined;
  const auto srcStage = lastUse ? lastUse->stage : vk::PipelineStageFlagBits2::eTopOfPipe;
  const auto srcAccess = lastUse ? lastUse->access : vk::AccessFlagBits2::eNone;

  if (oldLayout == ibp.layout && srcStage == ibp.stageFlags && srcAccess == ibp.accessFlags) {
    return std::nullopt;
  }

  return vk::ImageMemoryBarrier2{.srcStageMask = srcStage,
                                 .srcAccessMask = srcAccess,
                                 .dstStageMask = ibp.stageFlags,
                                 .dstAccessMask = ibp.accessFlags,
                                 .oldLayout = oldLayout,
                                 .newLayout = ibp.layout,
                                 .subresourceRange = vk::ImageSubresourceRange{
                                     .aspectMask = ibp.aspectFlags,
                                     .baseMipLevel = 0,
                                     .levelCount = 1,
                                     .baseArrayLayer = 0,
                                     .layerCount = 1,
                                 }};
}
}
