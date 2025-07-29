#include "Application.hpp"
#include "Preferences.hpp"
#include "api/fx/IEventQueue.hpp"
#include "api/fx/IGuiCallbackRegistrar.hpp"
#include "ui/Manager.hpp"

namespace ed {

Application::Application(std::shared_ptr<Preferences> newPreferences,
                         std::shared_ptr<Manager> newManager,
                         std::shared_ptr<tr::IEventQueue> newEventQueue,
                         std::shared_ptr<tr::IGuiCallbackRegistrar> newGuiCallbackRegistrar)
    : preferences{std::move(newPreferences)},
      manager{std::move(newManager)},
      eventQueue{std::move(newEventQueue)},
      guiCallbackRegistrar{std::move(newGuiCallbackRegistrar)} {

  Log.debug("Created Application");

  if (const auto recentFile = preferences->getRecentFile(); recentFile.has_value()) {
    Log.debug("recentFile from preferences: {0}", recentFile.value());
  }

  guiCallbackRegistrar->setRenderCallback([&](const tr::EditorState& editorState) {
    if (manager != nullptr) {
      manager->render(editorState);
    }
  });
}

Application::~Application() {
  Log.debug("Destroying Application");
}

auto Application::onStart() -> void {
  Log.trace("Application::onStart()");
}

auto Application::onUpdate() -> void {
  eventQueue->dispatchPending();
  // uiStateBuffer->pushState({})
}

auto Application::onShutdown() -> void {
  Log.trace("Application::onShutdown()");
}

}
