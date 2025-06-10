#pragma once

#include "r3/graph/ResourceAliases.hpp"
namespace tr {

struct ImageUsageInfo {
  ImageAlias alias;
  vk::AccessFlags2 accessFlags;
  vk::PipelineStageFlags2 stageFlags;
  vk::ImageAspectFlags aspectFlags;
  vk::ImageLayout layout;
  vk::ClearValue clearValue;
};

}
