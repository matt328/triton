#include "Application.hpp"
#include "Log.hpp"
#include "Game.hpp"

using Core::Log;

constexpr int width = 1366;
constexpr int height = 768;

int main() {
   Log::init();

   try {
      auto app = Application{width, height};
      auto resourceFactory = app.getResourceFactory();

      auto game = std::make_shared<game::Game>(resourceFactory, width, height);

      app.registerGame(game);

      app.run();

   } catch (const std::exception& e) { Log::game->error(e.what()); }
}
