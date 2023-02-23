#include "pch.h"

#include "log.h"
#include <spdlog/sinks/dist_sink.h>
#ifdef WIN32
#include <spdlog/sinks/msvc_sink.h>
#endif
#include <spdlog/sinks/stdout_color_sinks.h>

const std::string LOG_PATTERN = "%^%I:%M:%S %-8l %-11n: %v%$";

std::shared_ptr<spdlog::logger> Log::sCoreLogger;
std::shared_ptr<spdlog::logger> Log::sClientLogger;

void Log::init() {
   auto distSink = std::make_shared<spdlog::sinks::dist_sink_mt>();
   const auto stdoutColorSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
#ifdef WIN32
   const auto vsOutputSink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
   distSink->add_sink(vsOutputSink);
#endif

   distSink->add_sink(stdoutColorSink);
   distSink->set_pattern(LOG_PATTERN);

   sCoreLogger = std::make_shared<spdlog::logger>("Adventure2", distSink);
   sCoreLogger->set_level(spdlog::level::debug);

   sClientLogger = std::make_shared<spdlog::logger>("Application", distSink);
   sClientLogger->set_level(spdlog::level::debug);
}