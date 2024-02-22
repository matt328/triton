#pragma once

#include <entt/fwd.hpp>

namespace MM {
   template <class EntityType>
   class EntityEditor;
}

namespace Triton::Util {
   class Timer;
}

namespace Triton::Graphics {
   class Renderer;
}

namespace Triton::Actions {
   class ActionSystem;
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

      void fixedUpdate(const Util::Timer& timer);
      void update();
      void resize(const std::pair<uint32_t, uint32_t>);
      void waitIdle();

      // Input Handlers
      void keyCallback(int key, int scancode, int action, int mods);
      void cursorPosCallback(double xpos, double ypos);
      void mouseButtonCallback(int button, int action, int mods);
      void setMouseState(bool captured);

    private:
      std::unique_ptr<Graphics::Renderer> renderer;
      std::unique_ptr<entt::registry> registry;
      std::unique_ptr<Triton::Actions::ActionSystem> actionSystem;
      entt::entity room;
   };
}
