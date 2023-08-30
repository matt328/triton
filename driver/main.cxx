#include "Application.hpp"
#include "Logger.hpp"
#include "Game.hpp"
#include "config.h"

constexpr int width = 1366;
constexpr int height = 768;

int main() {
   Log::LogManager::getInstance().setMinLevel(Log::Level::Trace);

#ifdef _DEBUG
   Log::info << "Debug Build" << std::endl;
#else
   Log::info << "Release Build" << std::endl;
#endif

   auto windowTitle = std::string{PROJECT_NAME}.append(" - ").append(PROJECT_VER);

#ifdef _DEBUG
   windowTitle.append(" - Debug Build");
#else
   windowTitle.append(" - Release Build");
#endif

   try {
      auto app = Application{width, height, windowTitle};
      auto resourceFactory = app.getResourceFactory();

      auto game = std::make_shared<game::Game>(resourceFactory, width, height);

      app.registerGame(game);

      app.run();

   } catch (const std::exception& e) { Log::game->error(e.what()); }
}
