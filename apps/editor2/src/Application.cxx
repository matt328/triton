#include "Application.hpp"
#include "Properties.hpp"
#include "tr/IContext.hpp"
#include "tr/IGuiSystem.hpp"

namespace ed {

Application::Application(std::shared_ptr<Properties> newProperties,
                         std::shared_ptr<tr::IContext> newContext,
                         std::shared_ptr<tr::IGameplaySystem> newGameplaySystem,
                         std::shared_ptr<tr::IGuiSystem> newGuiSystem,
                         std::shared_ptr<Manager> newManager)
    : properties{std::move(newProperties)},
      context{std::move(newContext)},
      gameplaySystem{std::move(newGameplaySystem)},
      guiSystem{std::move(newGuiSystem)},
      manager{std::move(newManager)} {

  Log.debug("Created Application");

  if (const auto recentFile = properties->getRecentFile(); recentFile.has_value()) {
    Log.debug("recentFile from properties: {0}", recentFile.value().string());
  }

  guiSystem->setRenderCallback([&] {
    if (manager != nullptr) {
      manager->render();
    }
  });
}

Application::~Application() {
  Log.debug("Destroying Application");
}

void Application::run() const {
  Log.debug("Application run");
  context->run();
  Log.debug("Application stopped");
}

}
