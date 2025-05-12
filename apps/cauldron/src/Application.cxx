#include "Application.hpp"
#include "Properties.hpp"
#include "ui/Manager.hpp"

namespace ed {

Application::Application(std::shared_ptr<Properties> newProperties,
                         std::shared_ptr<Manager> newManager)
    : properties{std::move(newProperties)}, manager{std::move(newManager)} {

  Log.debug("Created Application");

  if (const auto recentFile = properties->getRecentFile(); recentFile.has_value()) {
    Log.debug("recentFile from properties: {0}", recentFile.value().string());
  }

  //   guiCallbackRegistrar->setRenderCallback([&] {
  //     if (manager != nullptr) {
  //       manager->render();
  //     }
  //   });
}

Application::~Application() {
  Log.debug("Destroying Application");
}

void Application::run() const {
  Log.debug("Application run");

  Log.debug("Application stopped");
}

}
