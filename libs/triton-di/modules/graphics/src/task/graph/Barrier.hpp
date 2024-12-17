#pragma once

namespace tr::gfx::task::graph {
   struct Barrier {
      vk::PipelineStageFlags srcStage;
      vk::PipelineStageFlags dstStage;
      vk::AccessFlags srcAccess;
      vk::AccessFlags dstAccess;

      vk::Buffer buffer;
      vk::DeviceSize offset{0};
      vk::DeviceSize size{vk::WholeSize};
   };
}