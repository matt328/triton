#pragma once

#include "IGame.hpp"
class RenderSystem;
class TransformSystem;
class InputSystem;
class IResourceFactory;

namespace game {

   class Game : public IGame {
    public:
      explicit Game(IResourceFactory* factory, int width, int height);
      ~Game() override;

      Game(const Game&) = delete;
      Game(Game&&) = delete;
      Game& operator=(const Game&) = delete;
      Game& operator=(Game&&) = delete;

      void update() override;
      void blendState(double alpha) override;
      void keyPressed(const int key, int scancode, const int action, int mods) override;

      std::vector<RenderObject> getRenderObjects() override;
      std::tuple<glm::mat4, glm::mat4, glm::mat4> getCameraParams() override;

    private:
      class GameImpl;
      std::unique_ptr<GameImpl> impl;
   };
}