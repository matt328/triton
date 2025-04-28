#pragma once

namespace tr {

struct ComputePassConfig {
  vk::raii::Pipeline pipeline;
  vk::raii::PipelineLayout pipelineLayout;
  std::optional<std::string> debugName = std::nullopt;
};

}
