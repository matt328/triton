#include "Application.hpp"
#include "Log.hpp"
#include "Game.hpp"
#include "config.h"

using Core::Log;

constexpr int width = 1366;
constexpr int height = 768;

int main() {
   Log::init();

#ifdef _DEBUG
   Log::core->info("Debug Build");
#else
   Log::core->info("NON Debug Build");
#endif

   try {
      auto app = Application{width, height};
      auto resourceFactory = app.getResourceFactory();

      auto game = std::make_shared<game::Game>(resourceFactory, width, height);

      app.registerGame(game);

      app.run();

   } catch (const std::exception& e) { Log::game->error(e.what()); }
}
