#pragma once

#include <spdlog/spdlog.h>

class Log {
 public:
   static void init();

   inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() {
      return sCoreLogger;
   }

   inline static std::shared_ptr<spdlog::logger>& GetClientLogger() {
      return sClientLogger;
   }

 private:
   static std::shared_ptr<spdlog::logger> sCoreLogger;
   static std::shared_ptr<spdlog::logger> sClientLogger;
};

#define LOG_ERROR(...) ::Log::GetClientLogger()->error(__VA_ARGS__)
#define LOG_WARN(...) ::Log::GetClientLogger()->warn(__VA_ARGS__)
#define LOG_INFO(...) ::Log::GetClientLogger()->info(__VA_ARGS__)
#define LOG_LDEBUG(...) ::Log::GetClientLogger()->debug(__VA_ARGS__)
#define LOG_TRACE(...) ::Log::GetClientLogger()->trace(__VA_ARGS__)
