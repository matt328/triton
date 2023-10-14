#include "ActionManager.hpp"
#include "ResourceFactory.hpp"
#include "ApplicationContext.hpp"
#include "Logger.hpp"

#include "config.h"

constexpr int width = 1366;
constexpr int height = 768;

using Triton::ApplicationContext;

int main() {
   Log::LogManager::getInstance().setMinLevel(Log::Level::Debug);

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
      auto appContext = ApplicationContext{width, height, windowTitle};

      auto actionManager = appContext.createActionManager();
      auto resourceFactory =
          appContext.createResourceFactory(std::filesystem::current_path() / "assets");

      const auto texture = resourceFactory->createTexture("some_texture_file");

      Log::debug << "Created Texture: " << texture << std::endl;

      appContext.start();

   } catch (const std::exception& e) { Log::error << e.what() << std::endl; }
}
