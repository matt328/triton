#pragma once

namespace Triton {
   class GameContainer {
    public:
      GameContainer(int width, int height) : width(width), height(height) {
      }
      virtual ~GameContainer() = default;

      GameContainer(const GameContainer&) = default;
      GameContainer(GameContainer&&) = delete;
      GameContainer& operator=(const GameContainer&) = default;
      GameContainer& operator=(GameContainer&&) = delete;

      void run() {
      }

    private:
      int width;
      int height;
   };
}