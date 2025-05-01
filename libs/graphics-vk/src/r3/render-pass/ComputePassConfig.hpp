#pragma once

namespace tr {

struct ComputePassConfig {
  std::string id;
  vk::raii::Pipeline pipeline;
  vk::raii::PipelineLayout pipelineLayout;
  std::optional<std::string> debugName = std::nullopt;
};

}
