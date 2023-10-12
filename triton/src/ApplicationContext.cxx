#include "ApplicationContext.hpp"
#include "Application.hpp"
#include "Renderer.hpp"
#include "ResourceManager.hpp"
#include "ActionManager.hpp"

namespace Triton {
   ApplicationContext::ApplicationContext(int width,
                                          int height,
                                          const std::string_view& windowTitle) {
      application = std::make_unique<Application>(width, height, windowTitle);

      renderer = std::make_shared<Renderer>(application->getWindow());
   }

   void start() {
   }
}