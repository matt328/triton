#pragma once

#include "mem/Buffer.hpp"

namespace tr::utils {
inline auto insertBarrier(const vk::raii::CommandBuffer& cmd, const Buffer& buffer) -> void {
  // Insert a memory barrier for the buffer the computeTask writes to
  vk::BufferMemoryBarrier bufferMemoryBarrier{
      .srcAccessMask = vk::AccessFlagBits::eShaderWrite,
      .dstAccessMask = vk::AccessFlagBits::eIndirectCommandRead,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .buffer = buffer.getBuffer(),
      .offset = 0,
      .size = VK_WHOLE_SIZE,
  };

  cmd.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader,
                      vk::PipelineStageFlagBits::eDrawIndirect,
                      vk::DependencyFlags{},
                      nullptr,
                      bufferMemoryBarrier,
                      nullptr);
}
}
