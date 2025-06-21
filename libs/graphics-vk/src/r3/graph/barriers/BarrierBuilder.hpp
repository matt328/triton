#pragma once

#include "r3/graph/barriers/BufferBarrierPrecursor.hpp"
#include "r3/graph/barriers/ImageBarrierPrecursor.hpp"
#include "r3/graph/barriers/LastImageUse.hpp"
#include "r3/graph/barriers/LastBufferUse.hpp"

namespace tr {

class BarrierBuilder {
public:
  BarrierBuilder(const BarrierBuilder&) = default;
  BarrierBuilder(BarrierBuilder&&) = delete;
  auto operator=(const BarrierBuilder&) -> BarrierBuilder& = default;
  auto operator=(BarrierBuilder&&) -> BarrierBuilder& = delete;

  static auto build(const BufferBarrierPrecursor& bbp, std::optional<LastBufferUse> lastUse)
      -> std::optional<vk::BufferMemoryBarrier2>;
  static auto build(const ImageBarrierPrecursor& ibp, std::optional<LastImageUse> lastUse)
      -> std::optional<vk::ImageMemoryBarrier2>;

private:
  BarrierBuilder() = default;
  ~BarrierBuilder() = default;
};

}
