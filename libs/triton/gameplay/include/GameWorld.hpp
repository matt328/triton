#pragma once

namespace tr::gp {
   class GameWorld {
    public:
      GameWorld() noexcept;
      ~GameWorld();

      GameWorld(const GameWorld&) = default;
      auto operator=(const GameWorld&) -> GameWorld& = default;

      GameWorld(GameWorld&&) = delete;
      auto operator=(GameWorld&&) -> GameWorld& = delete;
   };
} // namespace tr::gp