#pragma once
#include <string>

struct Logger {
   virtual void log(const std::string& message) = 0;
   virtual ~Logger() = default;
};