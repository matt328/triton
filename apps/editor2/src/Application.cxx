#include "Application.hpp"
#include "Properties.hpp"
#include "tr/IContext.hpp"

namespace ed {

   Application::Application(std::shared_ptr<Properties> newProperties,
                            std::shared_ptr<tr::IContext> newContext,
                            std::shared_ptr<tr::gp::IGameplaySystem> newGameplaySystem)
       : properties{std::move(newProperties)},
         context{std::move(newContext)},
         gameplaySystem{std::move(newGameplaySystem)} {

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
      context->run();
      Log.debug("Application stopped");
   }

}
