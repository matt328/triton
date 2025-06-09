#pragma once

namespace tr {

struct ComputePassConfig {
  uint8_t id;
  vk::raii::Pipeline pipeline;
  vk::raii::PipelineLayout pipelineLayout;
  std::optional<std::string> debugName = std::nullopt;
};

}
