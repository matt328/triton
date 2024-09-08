#include "config.h"
#include "Application.hpp"

#include "Logger2.hpp"

#undef TRACY_ENABLE

#if defined(TRACY_ENABLE)

void* operator new(const std::size_t count) {
   const auto ptr = malloc(count);
   TracyAllocS(ptr, count, 32);
   return ptr;
}
void operator delete(void* ptr) noexcept {
   TracyFreeS(ptr, 32);
   free(ptr);
}
#endif

#ifdef WIN32
int WINAPI WinMain([[maybe_unused]] _In_ HINSTANCE hInstance,
                   [[maybe_unused]] _In_opt_ HINSTANCE hPrevInstance,
                   [[maybe_unused]] _In_ LPSTR lpCmdLine,
                   [[maybe_unused]] _In_ int nShowCmd) {
#else
int main() {
#endif
   initLogger();

#ifdef _DEBUG
   Log.set_level(spdlog::level::trace);
#else
   Log.set_level(spdlog::level::warn);
#endif

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
      const auto app = std::make_unique<ed::Application>(width, height, ss.str());
      Log.info("Initialized");

      app->run();

   } catch (const std::exception& e) { Log.critical(e.what()); }
}