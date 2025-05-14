#include "Application.hpp"
#include "Properties.hpp"
#include "api/fx/IEventQueue.hpp"
#include "ui/Manager.hpp"

namespace ed {

Application::Application(std::shared_ptr<Properties> newProperties,
                         std::shared_ptr<Manager> newManager,
                         std::shared_ptr<tr::IEventQueue> newEventQueue)
    : properties{std::move(newProperties)},
      manager{std::move(newManager)},
      eventQueue{std::move(newEventQueue)} {

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

auto Application::onStart() -> void {
  Log.trace("Application::onStart()");
}

auto Application::onUpdate() -> void {
  eventQueue->dispatchPending();
}

auto Application::onShutdown() -> void {
  Log.trace("Application::onShutdown()");
}

}
