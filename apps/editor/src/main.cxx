#include "config.h"
#include "Application.hpp"

#include "Logger2.hpp"

#if defined(TRACY_ENABLE)

void* operator new(std::size_t count) {
   auto ptr = malloc(count);
   TracyAllocS(ptr, count, 32);
   return ptr;
}
void operator delete(void* ptr) noexcept {
   TracyFreeS(ptr, 32);
   free(ptr);
}
#endif

int main() {

   initLogger();

   Log.set_level(spdlog::level::trace);

   Log.info("Hello from spdlog");

   static constexpr int width = 1920;
   static constexpr int height = 1080;

#ifdef _DEBUG
   Log.info("Debug Build");
#else
   Log.info("Release Build");
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
      Log.info("Initialized");

      app->run();

   } catch (const std::exception& e) { Log.critical(e.what()); }
}