#include <futures/futures.hpp>
#include "config.h"
#include "Application.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "ImGuiSink.hpp"
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

   Log2.info("Hello from spdlog");

   return 0;

   // auto mySink = std::make_shared<my_sink_mt>();

   // auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

   // auto logger = spdlog::logger("basic", {mySink, console_sink});

   // logger.info("Hello from my logger with my sink");
   // logger.trace("Trace here");

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