#pragma once

namespace tr::cm {
   class IGuiSystem {
    public:
      IGuiSystem() = default;
      virtual ~IGuiSystem() = default;

      IGuiSystem(const IGuiSystem&) = default;
      IGuiSystem(IGuiSystem&&) = delete;
      auto operator=(const IGuiSystem&) -> IGuiSystem& = default;
      auto operator=(IGuiSystem&&) -> IGuiSystem& = delete;

      virtual void initialize() = 0;
   };
}