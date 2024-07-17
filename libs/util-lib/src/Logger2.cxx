#include "Logger2.hpp"

spdlog::logger Log2("basic");

void initLogger() {
   Log2 = spdlog::logger("basic");
}