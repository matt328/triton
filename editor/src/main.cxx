#include "config.h"
#include "Application.hpp"

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
      auto app = std::make_unique<ed::Application>(width, height, ss.str());
      Log::info << "Initialized" << std::endl;

      app->run();

   } catch (const std::exception& e) { Log::error << e.what() << std::endl; }
}