#include "ApplicationContext.hpp"
#include "Application.hpp"
#include "Renderer.hpp"
#include "ResourceFactory.hpp"
#include "ActionManager.hpp"

namespace Triton {
   class ApplicationContext::ApplicationContextImpl {
    public:
      ApplicationContextImpl(int width, int height, const std::string_view& windowTitle) {
         application = std::make_unique<Application>(width, height, windowTitle);
         renderer = std::make_shared<Renderer>(application->getWindow());
         actionManager = std::make_shared<Actions::ActionManager>();
      }

      void start() {
      }

      [[nodiscard]] std::shared_ptr<Actions::ActionManager> getActionManager() {
         return actionManager;
      }

    private:
      std::unique_ptr<Application> application;
      std::shared_ptr<Renderer> renderer;
      std::unique_ptr<ResourceFactory> resourceFactory;
      std::shared_ptr<Actions::ActionManager> actionManager;
   };

   ApplicationContext::ApplicationContext(int width,
                                          int height,
                                          const std::string_view& windowTitle) {
      impl = std::make_unique<ApplicationContextImpl>(width, height, windowTitle);
   }

   ApplicationContext::~ApplicationContext() = default;

   void ApplicationContext::start() {
      impl->start();
   }

   std::shared_ptr<Actions::ActionManager> ApplicationContext::getActionManager() {
      return impl->getActionManager();
   }
}