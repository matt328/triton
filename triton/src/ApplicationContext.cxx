#include "ApplicationContext.hpp"
#include "ActionManager.hpp"
#include "Application.hpp"
#include "DefaultResourceFactory.hpp"
#include "Renderer.hpp"
#include "ActionManager.hpp"

#include "Logger.hpp"
#include "events/ActionEvent.hpp"
#include "events/ApplicationEvent.hpp"
#include "Events.hpp"
#include "events/KeyEvent.hpp"

namespace Triton {
   class ApplicationContext::ApplicationContextImpl {
    public:
      ApplicationContextImpl(int width, int height, const std::string_view& windowTitle) {
         application = std::make_unique<Application>(width, height, windowTitle);
         renderer = std::make_shared<Renderer>(application->getWindow());

         application->addEventCallbackFn([this](Events::Event& e) { this->onEvent(e); });
      }

      size_t addEventHandler(std::function<void(Events::Event&)> fn) const {
         return application->addEventCallbackFn(std::move(fn));
      }

      void registerRenderObjectProvider(std::function<std::vector<RenderObject>()> fn) const {
         renderer->registerRenderObjectProvider(std::move(fn));
      }

      void registerPerFrameDataProvider(std::function<PerFrameData()> fn) const {
         renderer->registerPerFrameDataProvider(std::move(fn));
      }

      void onEvent(Events::Event& e) const {
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
            if (actionManager->hasMapping(e.getKey())) {
               const auto actionType = actionManager->mapKeyToAction(e.getKey());
               Events::ActionEvent event{actionType};
               application->fireEvent(event);
               return true;
            } else {
               return false;
            }
         });

         dispatcher.dispatch<Events::KeyReleasedEvent>(
             [this](Events::KeyReleasedEvent& e) { return true; });
      }

      void start() const {
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

   size_t ApplicationContext::addEventHandler(std::function<void(Events::Event&)> fn) {
      return impl->addEventHandler(fn);
   }

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