#pragma once

#include <spdlog/common.h>
#include <spdlog/spdlog.h>

extern spdlog::logger Log;

void initLogger(spdlog::level::level_enum debugLevel, spdlog::level::level_enum releaseLevel);
