#pragma once

#include "core/Timer.hpp"
#include "events/Events.hpp"
#include "game/actions/ActionManager.hpp"
/*
   Game's responsibility will be to own and handle interactions with the entity system
   Entity System
   - camera component - the camera from rust was a little off so grab a better one
   - transform component
   - Renderable component - just a pointer to a mesh loaded and owned by the Renderer

   - context variable for current camera
   - context variable for ActionStates
      - each system can check the ActionStates context variable

   - Movement System - opreates on Transform components
   - Camera System - moves cameras around based on ActionStates resource
   - Render System - sends data from camera and renderable components and transform components to
   the Renderer then calls renderer.render();
   - need to make sure the systems are updated in the correct update, and correct order
   - only poll input in preUpdate, update positions in fixedUpdate, and render in update()
*/

namespace Triton::Graphics {
   class Renderer;
}

namespace Triton::Game {
   class Game {
    public:
      Game(GLFWwindow* window);
      ~Game();

      Game(const Game&) = delete;
      Game(Game&&) = delete;
      Game& operator=(const Game&) = delete;
      Game& operator=(Game&&) = delete;

      void update(const Core::Timer& timer);
      void render();
      void resize(const int width, const int height);
      void waitIdle();
      void keyPressed(Actions::Key key);
      void keyReleased(Actions::Key key);

      [[nodiscard]] Actions::ActionManager& getActionManager() const {
         return *actionManager;
      }

    private:
      std::unique_ptr<Graphics::Renderer> renderer;

      std::unique_ptr<entt::registry> registry;

      std::unique_ptr<Actions::ActionManager> actionManager;
   };
}
