#pragma once

#include "Application.hpp"

namespace Triton {
   class GameContainer {
    public:
      GameContainer(int width, int height, const std::string_view& windowTitle);
      virtual ~GameContainer() = default;

      GameContainer(const GameContainer&) = delete;
      GameContainer(GameContainer&&) = delete;
      GameContainer& operator=(const GameContainer&) = delete;
      GameContainer& operator=(GameContainer&&) = delete;

      void run();

    protected:
      std::string createMesh(const std::string_view& filename);
      uint32_t createTexture(const std::string_view& filename);

    private:
      std::unique_ptr<Application> application;
   };

}