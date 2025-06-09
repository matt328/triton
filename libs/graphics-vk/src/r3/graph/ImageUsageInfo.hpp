#pragma once

#include "r3/graph/ResourceAliases.hpp"
namespace tr {

struct ImageUsageInfo {
  ResourceAlias alias;
  vk::AccessFlags accessFlags;
  vk::PipelineStageFlags stageFlags;
  vk::ImageAspectFlags aspectFlags;
  vk::ClearValue clearValue;
};

}
