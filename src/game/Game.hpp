#pragma once

#include "core/Timer.hpp"
#include "actions/ActionSystem.hpp"
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

      void beginFrame();
      void fixedUpdate(const Core::Timer& timer);
      void update();
      void resize(const int width, const int height);
      void waitIdle();
      void handleMessage(MSG msg);

      // Input Handlers
      void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    private:
      std::unique_ptr<Graphics::Renderer> renderer;
      std::unique_ptr<entt::registry> registry;
      std::unique_ptr<Actions::ActionSystem> actionSystem;
   };
}
