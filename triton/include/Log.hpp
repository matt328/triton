#pragma once

#include <memory>
#include <spdlog/spdlog.h>
#include "spdlog/sinks/base_sink.h"
#include <tracy/Tracy.hpp>

namespace Core {
   class Log {
    public:
      static void init();
      static std::shared_ptr<spdlog::logger> core;
      static std::shared_ptr<spdlog::logger> game;
   };

   template <typename Mutex>
   class TracySink : public spdlog::sinks::base_sink<Mutex> {
    protected:
      void sink_it_(const spdlog::details::log_msg& msg) override {
         TracyMessage(msg.payload.data(), msg.payload.size());
      }

      void flush_() override {
      }
   };
}