#pragma once

namespace Triton {

   class Renderer;
   class ResourceManager;
   class Application;
   class ActionManager;

   class ApplicationContext {
    public:
      ApplicationContext(int width, int height, const std::string_view& windowTitle);

      void start();

    private:
      std::unique_ptr<Application> application;
      std::shared_ptr<Renderer> renderer;
      std::unique_ptr<ResourceManager> resourceManager;
      std::unique_ptr<ActionManager> actionManager;
   };

};
