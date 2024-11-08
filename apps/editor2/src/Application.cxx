#include "Application.hpp"
#include "Properties.hpp"
#include "cm/Timer.hpp"

namespace ed {

   Application::Application(std::shared_ptr<Properties> newProperties,
                            std::shared_ptr<tr::cm::Timer> newTimer)
       : properties{std::move(newProperties)}, timer{std::move(newTimer)} {

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
