#include "Context.hpp"

namespace tr {
   Context::Context(void* nativeWindow) {
      game = std::make_unique<gp::Game>(static_cast<GLFWwindow*>(nativeWindow));
      renderer = std::make_unique<gfx::Renderer>(static_cast<GLFWwindow*>(nativeWindow));
   }
}