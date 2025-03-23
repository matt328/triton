#pragma once

#include "cm/IGuiAdapter.hpp"

namespace tr {
class ImGuiAdapter : public IGuiAdapter {
public:
  ImGuiAdapter() = default;
  ~ImGuiAdapter() override = default;

  ImGuiAdapter(const ImGuiAdapter&) = default;
  ImGuiAdapter(ImGuiAdapter&&) = delete;
  auto operator=(const ImGuiAdapter&) -> ImGuiAdapter& = default;
  auto operator=(ImGuiAdapter&&) -> ImGuiAdapter& = delete;

  auto needsKeyboard() -> bool override;
  auto needsMouse() -> bool override;
  auto disableMouse() -> void override;
  auto enableMouse() -> void override;
};
}
