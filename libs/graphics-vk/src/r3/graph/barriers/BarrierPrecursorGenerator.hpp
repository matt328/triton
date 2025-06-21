#pragma once

#include "r3/graph/barriers/BarrierPrecursorPlan.hpp"
#include "r3/render-pass/IRenderPass.hpp"

namespace tr {

class BarrierPrecursorGenerator {
public:
  BarrierPrecursorGenerator() = default;
  ~BarrierPrecursorGenerator() = default;

  BarrierPrecursorGenerator(const BarrierPrecursorGenerator&) = default;
  BarrierPrecursorGenerator(BarrierPrecursorGenerator&&) = delete;
  auto operator=(const BarrierPrecursorGenerator&) -> BarrierPrecursorGenerator& = default;
  auto operator=(BarrierPrecursorGenerator&&) -> BarrierPrecursorGenerator& = delete;

  auto build(const std::vector<std::unique_ptr<IRenderPass>>& passes) -> BarrierPrecursorPlan;
};

}
