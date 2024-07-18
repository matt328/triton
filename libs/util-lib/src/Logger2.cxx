#include "Logger2.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

spdlog::logger Log("basic");

const std::string LOG_PATTERN = "%^%I:%M:%S | %-5l | %-6n | %v%$";

void initLogger() {
   auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
   console_sink->set_pattern(LOG_PATTERN);
   Log = spdlog::logger("basic", console_sink);
}