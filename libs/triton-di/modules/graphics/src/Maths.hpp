#pragma once

namespace tr::maths {
inline static auto scaleNumber(const uint32_t number, const float percent) -> uint32_t {
  const auto scaledNumber = static_cast<float>(number) * percent;
  float whole = 0.f;
  std::modf(scaledNumber, &whole);
  return static_cast<uint32_t>(whole);
}
}
