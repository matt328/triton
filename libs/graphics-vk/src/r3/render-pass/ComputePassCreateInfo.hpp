#pragma once

#include "r3/render-pass/PipelineCreateInfo.hpp"
namespace tr {

struct ComputePassCreateInfo {
  uint8_t id;
  PipelineLayoutInfo pipelineLayoutInfo;
  ShaderStageInfo shaderStageInfo;
};

}
