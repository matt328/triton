#pragma once

#include <iostream>
#include <type_traits>
#include <ctime>
#include <iomanip>
#include <string_view>

namespace Log {

   enum class Level {
      Trace = 0,
      Debug,
      Info,
      Warn,
      Error
   };

   class LogManager {
    public:
      static LogManager& getInstance() {
         static LogManager instance;
         return instance;
      }
      void setMinLevel(Level newLevel) {
         level = newLevel;
      }

      [[nodiscard]] Level getLevel() const {
         return level;
      }

    private:
      LogManager() = default;
      Level level;
   };

   class Logger {
    public:
      Logger(Level level) : level(level) {
      }

      [[nodiscard]] const std::string_view header() const {

         if (level == Level::Trace) {
            return "Trace";
         }
         if (level == Level::Debug) {
            return "\033[1;34mDebug\033[0m";
         }
         if (level == Level::Info) {
            return "\033[1;32mInfo \033[0m";
         }
         if (level == Level::Warn) {
            return "\033[1;33mWarn \033[0m";
         }
         if (level == Level::Error) {
            return "\033[1;31mError\033[0m";
         }
         return "Unknown";
      }

      template <typename T>
      Logger& operator<<(const T& value) {
         if (LogManager::getInstance().getLevel() > level) {
            return *this;
         }
         if (isNextBegin) {
            auto t = std::time(nullptr);
            auto tm = *std::localtime(&t);
            std::cout << std::put_time(&tm, "%I:%M:%S") << " | " << header() << " | " << value;
         } else {
            std::cout << value;
         }
         isNextBegin = false;
         return *this;
      }

      Logger& operator<<(decltype(std::endl<char, std::char_traits<char>>)& manip) {
         if (LogManager::getInstance().getLevel() > level) {
            return *this;
         }
         isNextBegin = true;
         std::cout << manip;
         return *this;
      }

    private:
      Level level;
      bool isNextBegin{true};
   };

   inline Logger trace = Logger{Level::Trace};
   inline Logger debug = Logger{Level::Debug};
   inline Logger info = Logger{Level::Info};
   inline Logger warn = Logger{Level::Warn};
   inline Logger error = Logger{Level::Error};

}