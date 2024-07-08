#pragma once

namespace tr::gp {
   class GameWorld {
    public:
      GameWorld() noexcept;
      ~GameWorld();

      GameWorld(const GameWorld&) = default;
      GameWorld& operator=(const GameWorld&) = default;

      GameWorld(GameWorld&&) = delete;
      GameWorld& operator=(GameWorld&&) = delete;
   };
}