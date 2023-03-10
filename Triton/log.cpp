#include "Log.h"

#include <spdlog/sinks/dist_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

const std::string LOG_PATTERN = "%^%I:%M:%S | %-5l | %-6n | %v%$";

std::shared_ptr<spdlog::logger> Log::core;
std::shared_ptr<spdlog::logger> Log::game;

void Log::init() {
   auto distSink = std::make_shared<spdlog::sinks::dist_sink_mt>();
   const auto stdoutColorSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
   const auto vsOutputSink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
   distSink->add_sink(vsOutputSink);

   distSink->add_sink(stdoutColorSink);
   distSink->set_pattern(LOG_PATTERN);

   core = std::make_shared<spdlog::logger>("game", distSink);
   core->set_level(spdlog::level::debug);

   game = std::make_shared<spdlog::logger>("triton", distSink);
   game->set_level(spdlog::level::debug);
}
