#include "Logger2.hpp"
#include <spdlog/async.h>
#include <spdlog/async_logger.h>
#include <spdlog/sinks/basic_file_sink.h>

spdlog::logger Log("basic");

const std::string LOG_PATTERN = "%^%I:%M:%S %-8l %-8n %v%$";

void initLogger([[maybe_unused]] spdlog::level::level_enum debugLevel,
                [[maybe_unused]] spdlog::level::level_enum releaseLevel) {
  const auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_pattern(LOG_PATTERN);

  spdlog::init_thread_pool(8192, 1);

  Log = spdlog::async_logger("core",
                             console_sink,
                             spdlog::thread_pool(),
                             spdlog::async_overflow_policy::block);
#ifdef _DEBUG
  Log.set_level(debugLevel);
#else
  Log.set_level(releaseLevel);
#endif
}

void LogTest(std::source_location location) {
  Log.debug("{}:{} log message here", location.file_name(), location.line());
}
