#pragma once

namespace tr::gp {
   class IGameplaySystem {
    public:
      explicit IGameplaySystem() = default;
      virtual ~IGameplaySystem() = default;

      virtual void update() = 0;
      virtual void fixedUpdate() = 0;
   };
}