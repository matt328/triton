#pragma once

#include <memory>
#include <spdlog/spdlog.h>

namespace Core {
   class Log {
    public:
      static void init();
      static std::shared_ptr<spdlog::logger> core;
      static std::shared_ptr<spdlog::logger> game;
   };
}