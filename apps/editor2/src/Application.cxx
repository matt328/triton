#include "Application.hpp"
#include "Properties.hpp"
#include "cm/Timer.hpp"
#include "cm/event/EventBus.hpp"
#include "cm/IWindow.hpp"

namespace ed {

   Application::Application(std::shared_ptr<tr::cm::IWindow> newWindow,
                            std::shared_ptr<Properties> newProperties,
                            [[maybe_unused]] tr::cm::evt::EventBus& eventBus,
                            std::shared_ptr<tr::cm::Timer> newTimer)
       : window(std::move(newWindow)),
         properties{std::move(newProperties)},
         timer{std::move(newTimer)} {

      Log.debug("Created Application");

      const auto recentFile = properties->getRecentFile();
      if (recentFile.has_value()) {
         Log.debug("recentfile from properties: {0}", recentFile.value().string());
      }
   }

   Application::~Application() {
      Log.debug("Destroying Application");
   }

   void Application::run() const {
      Log.debug("Application run");
      timer->start();
      Log.debug("Application stopped");
   }

}
