#pragma once

namespace tr::cm {
   class IGuiSystem {
    public:
      IGuiSystem() = default;
      ~IGuiSystem() = default;

      virtual void initialize() = 0;
   };
}