#pragma once

#include "Events.hpp"
#include "TransferData.hpp"

class RenderSystem;
class TransformSystem;
class IResourceFactory;

namespace game {

   class Game {
    public:
      explicit Game(IResourceFactory* factory, int width, int height);
      ~Game();

      Game(const Game&) = delete;
      Game(Game&&) = delete;
      Game& operator=(const Game&) = delete;
      Game& operator=(Game&&) = delete;

      void update();
      void blendState(double alpha);

      std::vector<Triton::RenderObject> getRenderObjects();
      std::tuple<glm::mat4, glm::mat4, glm::mat4> getCameraParams();

      void onEvent(Events::Event& e);

    private:
      class GameImpl;
      std::unique_ptr<GameImpl> impl;
   };

}