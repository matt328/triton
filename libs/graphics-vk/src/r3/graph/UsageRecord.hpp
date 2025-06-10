#pragma once

namespace tr {

struct UsageRecord {
  size_t passIndex;
  bool isWrite;
  vk::AccessFlags accessFlags;
  vk::PipelineStageFlags stageFlags;
};

}
