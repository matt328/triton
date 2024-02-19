#include "config.h"
#include "core/Timer.hpp"
#include "game/Application.hpp"

void* operator new(std ::size_t count) {
   auto ptr = malloc(count);
   TracyAllocS(ptr, count, 12);
   return ptr;
}
void operator delete(void* ptr) noexcept {
   TracyFreeS(ptr, 12);
   free(ptr);
}

int main() {
   Log::LogManager::getInstance().setMinLevel(Log::Level::Trace);

   static constexpr int TARGET_FPS = 60;
   static constexpr int MAX_UPDATES = 4;

   static constexpr int width = 1920;
   static constexpr int height = 1080;

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
      auto timer = Triton::Core::Timer(TARGET_FPS, MAX_UPDATES);

      auto app = std::make_unique<Triton::Game::Application>(width, height, windowTitle);
      Log::info << "Initialized" << std::endl;

      app->run(timer);

   } catch (const std::exception& e) { Log::error << e.what() << std::endl; }
}
