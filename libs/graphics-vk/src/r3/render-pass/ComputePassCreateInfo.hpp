#pragma once

#include "r3/render-pass/PipelineCreateInfo.hpp"
namespace tr {

struct ComputePassCreateInfo {
  std::string id;
  PipelineLayoutInfo pipelineLayoutInfo;
  ShaderStageInfo shaderStageInfo;
};

}
