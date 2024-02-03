#include "config.h"
#include "core/Timer.hpp"
#include "game/Game.hpp"

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
      bool running = true;
      auto timer = Triton::Core::Timer(60, 4);

      auto game = std::make_unique<Triton::Game::Game>();
      Log::info << "Initialized" << std::endl;

      while (running) {
         timer.tick([&]() { game->update(timer); });
      }

   } catch (const std::exception& e) { Log::error << e.what() << std::endl; }
}
