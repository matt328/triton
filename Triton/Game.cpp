#include "Game.h"
#include "Log.h"

void Game::update(const float t, const float dt) {
   // previousState = currentState;
   // Update state
   Log::core->debug("update() t: {}, dt: {}", t, dt);
}

void Game::blendState(double alpha) {

   // state = currentState * alpha + previousState * (1.0 - alpha);

   Log::core->debug("blendState(), alpha: {}", alpha);
}
