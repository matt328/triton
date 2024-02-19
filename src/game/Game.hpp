#pragma once

#include "core/Timer.hpp"
#include "actions/ActionSystem.hpp"

#include "graphics/gui/EntityEditor.hpp"
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
      void keyCallback(int key, int scancode, int action, int mods);
      void cursorPosCallback(double xpos, double ypos);
      void mouseButtonCallback(int button, int action, int mods);
      void setMouseState(bool captured);

    private:
      std::unique_ptr<Graphics::Renderer> renderer;
      std::unique_ptr<entt::registry> registry;
      std::unique_ptr<Actions::ActionSystem> actionSystem;
      std::unique_ptr<MM::EntityEditor<entt::entity>> entityEditor;
      entt::entity room;
   };
}
