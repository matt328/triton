#pragma once

#include "IGuiSystem.hpp"

namespace tr::cm {
   class ImGuiSystem : public IGuiSystem {
    public:
      ImGuiSystem() = default;
      ~ImGuiSystem() override = default;

      ImGuiSystem(const ImGuiSystem&) = default;
      ImGuiSystem(ImGuiSystem&&) = delete;
      auto operator=(const ImGuiSystem&) -> ImGuiSystem& = default;
      auto operator=(ImGuiSystem&&) -> ImGuiSystem& = delete;

      void initialize() override;

    private:
      bool isInitialized{};
   };
}