#pragma once

#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <source_location>

extern spdlog::logger Log;

void initLogger(spdlog::level::level_enum debugLevel, spdlog::level::level_enum releaseLevel);

void LogTest(std::source_location location = std::source_location::current());
