#include "ResourceFactory.hpp"
#include "ApplicationContext.hpp"
#include "Logger.hpp"

#include "events/ActionEvent.hpp"
#include "game/LayerStack.hpp"
#include "game/FirstLayer.hpp"

#include "config.h"

constexpr int width = 1366;
constexpr int height = 768;

using Triton::ApplicationContext;

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
      auto appContext = ApplicationContext{width, height, windowTitle};
      const auto actionManager = appContext.createActionManager();

      const auto layerStack = std::make_unique<Game::LayerStack>(actionManager);
      const auto id = layerStack->pushNew<Game::FirstLayer>();
      layerStack->switchTo(id);

      appContext.addEventHandler(
          [&layerStack](Triton::Events::Event& event) { layerStack->handleEvent(event); });

      const auto resourceFactory =
          appContext.createResourceFactory(std::filesystem::current_path() / "assets");

      const auto texture = resourceFactory->createTexture("some_texture_file");
      Log::debug << "Created Texture: " << texture << std::endl;

      const auto mesh = resourceFactory->createMesh("some_mesh_file");

      appContext.start();

   } catch (const std::exception& e) { Log::error << e.what() << std::endl; }
}
