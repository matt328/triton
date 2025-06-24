#pragma once

#include "api/fx/IStaticStateBuffer.hpp"
#include "UIState.hpp"

namespace ed {

class UIStateBuffer : public tr::IStaticStateBuffer<UIState> {
public:
  UIStateBuffer() = default;
  ~UIStateBuffer() = default;

  UIStateBuffer(const UIStateBuffer&) = default;
  UIStateBuffer(UIStateBuffer&&) = delete;
  auto operator=(const UIStateBuffer&) -> UIStateBuffer& = default;
  auto operator=(UIStateBuffer&&) -> UIStateBuffer& = delete;

  auto getStates(tr::Timestamp t) -> std::optional<UIState> override;
  auto pushState(const UIState& newState, tr::Timestamp t) -> void override;
};

}
