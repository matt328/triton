#pragma once

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

      [[nodiscard]] const std::string_view header(bool color = true) const {

         if (level == Level::Trace) {
            return "Trace";
         }
         if (level == Level::Debug) {
            if (color) {
               return "\033[1;34mDebug\033[0m";
            } else {
               return "Debug";
            }
         }
         if (level == Level::Info) {
            if (color) {
               return "\033[1;32mInfo \033[0m";
            } else {
               return "Info ";
            }
         }
         if (level == Level::Warn) {
            if (color) {
               return "\033[1;33mWarn \033[0m";
            } else {
               return "Warn ";
            }
         }
         if (level == Level::Error) {
            if (color) {
               return "\033[1;31mError\033[0m";
            } else {
               return "Error";
            }
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
            if (sink) {
               ss << std::put_time(&tm, "%I:%M:%S") << " | " << header(false) << " | " << value;
            }
            std::cout << std::put_time(&tm, "%I:%M:%S") << " | " << header() << " | " << value;
         } else {
            if (sink) {
               ss << value;
            }
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
         if (sink) {
            ss << manip;
            sink(ss.str());
            ss.clear();
         }
         std::cout << manip;
         return *this;
      }

      void addSink(const std::function<void(std::string)>& fn) {
         sink = fn;
      }

    private:
      Level level;
      bool isNextBegin{true};
      std::function<void(std::string)> sink = nullptr;
      std::stringstream ss;
   };

   // I'd like to make these const but because of L82 they can't be
   inline Logger trace = Logger{Level::Trace};
   inline Logger debug = Logger{Level::Debug};
   inline Logger info = Logger{Level::Info};
   inline Logger warn = Logger{Level::Warn};
   inline Logger error = Logger{Level::Error};

}