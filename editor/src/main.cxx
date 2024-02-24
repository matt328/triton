#include "config.h"
#include "util/Timer.hpp"
#include "Application.hpp"

/*
   TODO:
   - pull whole Game namespace into another lib
      - game namespace will contain ecs, scripting system, input mappings
      - try to keep it generic so that editor project can be used to create a game file
        that the game ns can load and run autonomously when in an executable game
      - come up with some other name other than game
      - editor and <game executable> will both depend on it
   - Remove refs to glfw from triton

*/

#if defined(TRACY_ENABLE)
void* operator new(std::size_t count) {
   auto ptr = malloc(count);
   TracyAllocS(ptr, count, 12);
   return ptr;
}
void operator delete(void* ptr) noexcept {
   TracyFreeS(ptr, 12);
   free(ptr);
}
#endif

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

   auto ss = std::stringstream{};
   ss << PROJECT_NAME << " v" << PROJECT_VER;

#ifdef _DEBUG

   ss << " - Debug Build";
#else
   ss << " - Release Build";
#endif

   try {
      auto timer = Triton::Util::Timer(TARGET_FPS, MAX_UPDATES);

      auto app = std::make_unique<Triton::Application>(width, height, ss.str());
      Log::info << "Initialized" << std::endl;

      app->run(timer);

   } catch (const std::exception& e) { Log::error << e.what() << std::endl; }
}