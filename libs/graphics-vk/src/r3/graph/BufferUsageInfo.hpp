#pragma once

#include "r3/graph/ResourceAliases.hpp"

namespace tr {
struct BufferUsageInfo {
  ResourceAlias alias;
  vk::AccessFlags2 accessFlags;
  vk::PipelineStageFlags2 stageFlags;
  vk::DeviceSize offset = 0;
  vk::DeviceSize size = VK_WHOLE_SIZE;
};
}
