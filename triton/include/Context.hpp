#pragma once

/* Project Architecture Notes
   Modules
   - triton
      - renderer frontend
      - renderer backend (vulkan)
      - gameplay (ecs)
   - editor
      - Application
      - renderers for editor components
      - editor adapter that can be installed into the renderer via the frontend, allows the editor
        components to be rendered by the renderer backend.
      - editor core - handles communication between editor components and gameplay
   - game
      - Application
      - calls gameplay.runGame()
      - callbacks for handling saving game state, windowing, etc.

*/

namespace tr {
   class Context {
    public:
      Context(void* nativeWindow);
      ~Context() = default;

      Context(const Context&) = default;
      Context(Context&&) = delete;
      Context& operator=(const Context&) = default;
      Context& operator=(Context&&) = delete;

      void start();

    private:
      std::unique_ptr<gp::Game> game;
      std::unique_ptr<gfx::Renderer> renderer;
   };
}