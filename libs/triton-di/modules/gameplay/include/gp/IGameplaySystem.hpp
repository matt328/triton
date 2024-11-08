#pragma once

namespace tr::gp {
   class IGameplaySystem {
    public:
      explicit IGameplaySystem() = default;
      virtual ~IGameplaySystem() = default;

      IGameplaySystem(const IGameplaySystem&) = default;
      IGameplaySystem(IGameplaySystem&&) = delete;
      auto operator=(const IGameplaySystem&) -> IGameplaySystem& = default;
      auto operator=(IGameplaySystem&&) -> IGameplaySystem& = delete;

      virtual void update() = 0;
      virtual void fixedUpdate() = 0;
   };
}