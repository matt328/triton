#include "ApplicationContext.hpp"
#include "Application.hpp"
#include "Renderer.hpp"
#include "ResourceManager.hpp"
#include "actions/ActionManager.hpp"

namespace Triton {
   class ApplicationContext::ApplicationContextImpl {
    public:
      ApplicationContextImpl(int width, int height, const std::string_view& windowTitle) {
         application = std::make_unique<Application>(width, height, windowTitle);
         renderer = std::make_shared<Renderer>(application->getWindow());
      }

      void start() {
      }

    private:
      std::unique_ptr<Application> application;
      std::shared_ptr<Renderer> renderer;
      std::unique_ptr<ResourceManager> resourceManager;
      std::unique_ptr<Actions::ActionManager> actionManager;
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
}