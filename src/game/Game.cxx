#include "Game.hpp"

namespace Triton::Game {

   void Game::update([[maybe_unused]] const Core::Timer& timer) {
      render();
   }

   void Game::render() {
   }
}