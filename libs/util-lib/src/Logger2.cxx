#include "Logger2.hpp"

spdlog::logger Log("basic");

const std::string LOG_PATTERN = "%^%I:%M:%S %-8l %-8n %v%$";

void initLogger() {
   const auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
   console_sink->set_pattern(LOG_PATTERN);
   Log = spdlog::logger("core", console_sink);
}