#pragma once

namespace tr {
struct PushConstantBlob {
  std::vector<uint8_t> data;
  vk::ShaderStageFlags stageFlags;
  uint32_t offset = 0;
};
}
