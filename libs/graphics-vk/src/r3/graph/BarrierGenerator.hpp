#pragma once

#include "BarrierData.hpp"
#include "r3/ComponentIds.hpp"

namespace tr {

class IRenderPass;

struct BarrierPlan {
  std::unordered_map<PassId, std::vector<ImageBarrierData>> imageBarriers;
  std::unordered_map<PassId, std::vector<BufferBarrierData>> bufferBarriers;
};

struct LastImageUse {
  PassId passId;
  vk::AccessFlags2 accessMask;
  vk::PipelineStageFlags2 stageMask;
  vk::ImageLayout layout;
};

struct LastBufferUse {
  PassId passId;
  vk::AccessFlags2 accessMask;
  vk::PipelineStageFlags2 stageMask;
};

class BarrierGenerator {
public:
  BarrierGenerator() = default;
  ~BarrierGenerator() = default;

  BarrierGenerator(const BarrierGenerator&) = default;
  BarrierGenerator(BarrierGenerator&&) = delete;
  auto operator=(const BarrierGenerator&) -> BarrierGenerator& = default;
  auto operator=(BarrierGenerator&&) -> BarrierGenerator& = delete;

  auto build(const std::vector<std::unique_ptr<IRenderPass>>& passes) -> BarrierPlan;

private:
  std::unordered_map<ImageAlias, LastImageUse> lastImageUses;
  std::unordered_map<BufferAliasVariant, LastBufferUse> lastBufferUses;
};

}
