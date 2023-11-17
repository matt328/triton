#include "ApplicationContext.hpp"
#include "ActionManager.hpp"
#include "Application.hpp"
#include "DefaultResourceFactory.hpp"
#include "Renderer.hpp"
#include "ActionManager.hpp"

#include "Logger.hpp"
#include "events/ApplicationEvent.hpp"
#include "events/Events.hpp"
#include "events/KeyEvent.hpp"

namespace Triton {
   class ApplicationContext::ApplicationContextImpl {
    public:
      ApplicationContextImpl(int width, int height, const std::string_view& windowTitle) {
         application = std::make_unique<Application>(width, height, windowTitle);
         renderer = std::make_shared<Renderer>(application->getWindow());

         application->setEventCallbackFn([this](Events::Event& e) { this->onEvent(e); });
      }

      void registerRenderObjectProvider(std::function<std::vector<RenderObject>()> fn) {
         renderer->registerRenderObjectProvider(fn);
      }

      void registerPerFrameDataProvider(std::function<PerFrameData()> fn) {
         renderer->registerPerFrameDataProvider(fn);
      }

      void onEvent(Events::Event& e) {
         auto dispatcher = Events::EventDispatcher{e};

         dispatcher.dispatch<Events::RenderEvent>([this](Events::RenderEvent& e) {
            this->renderer->render();
            return true;
         });

         dispatcher.dispatch<Events::ShutdownEvent>([this](Events::ShutdownEvent& e) {
            this->renderer->waitIdle();
            return true;
         });

         dispatcher.dispatch<Events::FixedUpdateEvent>(
             [](Events::FixedUpdateEvent& e) { return true; });

         dispatcher.dispatch<Events::UpdateEvent>([](Events::UpdateEvent& e) { return true; });

         dispatcher.dispatch<Events::KeyPressedEvent>([this](Events::KeyPressedEvent& e) {
            actionManager->keyPressed(e.getKey());
            return true;
         });

         dispatcher.dispatch<Events::KeyReleasedEvent>([this](Events::KeyReleasedEvent& e) {
            actionManager->keyReleased(e.getKey());
            return true;
         });
      }

      void start() {
         application->run();
      }

      [[nodiscard]] std::shared_ptr<Actions::ActionManager> createactionManager() {
         actionManager = std::make_shared<Actions::ActionManager>();
         return actionManager;
      }

      [[nodiscard]] std::shared_ptr<ResourceFactory> createResourceFactory(
          std::filesystem::path rootPath) {
         resourceFactory = std::make_shared<DefaultResourceFactory>(rootPath);
         return resourceFactory;
      }

    private:
      std::unique_ptr<Application> application;
      std::shared_ptr<Renderer> renderer;
      std::shared_ptr<ResourceFactory> resourceFactory;
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

   std::shared_ptr<Actions::ActionManager> ApplicationContext::createActionManager() {
      return impl->createactionManager();
   }

   std::shared_ptr<ResourceFactory> ApplicationContext::createResourceFactory(
       std::filesystem::path rootPath) {
      return impl->createResourceFactory(rootPath);
   }

   void ApplicationContext::registerRenderObjectProvider(
       std::function<std::vector<RenderObject>()> fn) {
      impl->registerRenderObjectProvider(fn);
   }

   void ApplicationContext::registerPerFrameDataProvider(std::function<PerFrameData()> fn) {
      impl->registerPerFrameDataProvider(fn);
   }

}