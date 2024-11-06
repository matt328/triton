// logger_impl.h
#pragma once
#include "Logger.hpp"
#include <iostream>

class LoggerImpl : public Logger {
 public:
   void log(const std::string& message) override {
      std::cout << "Log: " << message << std::endl;
   }
};