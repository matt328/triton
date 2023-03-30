#include "Log.hpp"

namespace Core {
   const std::string LOG_PATTERN = "%^%I:%M:%S | %-5l | %-6n | %v%$";

   std::shared_ptr<spdlog::logger> Log::core;
   std::shared_ptr<spdlog::logger> Log::game;

   void Log::init() {
      auto distSink = std::make_shared<spdlog::sinks::dist_sink_mt>();
      const auto stdoutColorSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

#ifdef WIN32
      const auto vsOutputSink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
      distSink->add_sink(vsOutputSink);
#endif

      distSink->add_sink(stdoutColorSink);
      distSink->set_pattern(LOG_PATTERN);

      core = std::make_shared<spdlog::logger>("triton", distSink);
      core->set_level(spdlog::level::debug);

      game = std::make_shared<spdlog::logger>("game", distSink);
      game->set_level(spdlog::level::debug);
   }
}