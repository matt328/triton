#include "config.h"

int main() {
   Log::LogManager::getInstance().setMinLevel(Log::Level::Trace);

#ifdef _DEBUG
   Log::info << "Debug Build" << std::endl;
#else
   Log::info << "Release Build" << std::endl;
#endif

   auto windowTitle = std::string{PROJECT_NAME}.append(" - ").append(PROJECT_VER);

#ifdef _DEBUG

   windowTitle.append(" - Debug Build");
#else
   windowTitle.append(" - Release Build");
#endif

   try {
      Log::info << "Initialized" << std::endl;
   } catch (const std::exception& e) { Log::error << e.what() << std::endl; }
}
