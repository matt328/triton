#include "Application.hpp"
#include "GameContainer.hpp"
#include "Logger.hpp"
#include "Game.hpp"
#include "config.h"
#include "GameContainer.hpp"

constexpr int width = 1366;
constexpr int height = 768;

using Triton::GameContainer;

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

   class MyGame : public GameContainer {
    public:
      MyGame(int width, int height, const std::string_view& windowTitle) :
          GameContainer(width, height, windowTitle) {
         Log::debug << "Created MyGame" << std::endl;
      }
   };

   auto myGame = MyGame{width, height, windowTitle};

   myGame.run();

   try {
      auto app = Application{width, height, windowTitle};
      auto resourceFactory = app.getResourceFactory();

      auto game = std::make_unique<game::Game>(resourceFactory, width, height);

      app.setEventCallbackFn([&game](Events::Event& e) -> void { game->onEvent(e); });
      app.registerRenderObjectProvider([&game]() { return game->getRenderObjects(); });
      app.registerPerFrameDataProvider([&game]() { return game->getCameraParams(); });

      app.run();

   } catch (const std::exception& e) { Log::error << e.what() << std::endl; }
}
