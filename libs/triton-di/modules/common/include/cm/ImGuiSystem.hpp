#pragma once

#include "IGuiSystem.hpp"
namespace tr::cm {
   class ImGuiSystem : public IGuiSystem {
    public:
      ImGuiSystem() = default;
      ~ImGuiSystem() = default;

      void initialize() override;

    private:
      bool isInitialized{};
   };
}