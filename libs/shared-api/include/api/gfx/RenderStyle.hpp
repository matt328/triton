#pragma once

namespace tr {

enum class RenderPassType : uint32_t {
  GBuffer = 1 << 0,
  MotionVectors = 1 << 1,
  DeferredLighting = 1 << 2,
  Forward = 1 << 3,
  Wireframe = 1 << 4,
  Overlay = 1 << 5,
  Bloom = 1 << 6,
  TAA = 1 << 8,
  Tonemapping = 1 << 9
};

constexpr auto operator|(RenderPassType lhs, RenderPassType rhs) -> RenderPassType {
  return static_cast<RenderPassType>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr auto operator&(RenderPassType lhs, RenderPassType rhs) -> RenderPassType {
  return static_cast<RenderPassType>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr auto any(RenderPassType mask, RenderPassType test) -> bool {
  return static_cast<uint32_t>(mask & test) != 0;
}

}
