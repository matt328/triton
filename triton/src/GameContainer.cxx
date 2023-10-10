#include "GameContainer.hpp"

namespace Triton {
   GameContainer::GameContainer(int width, int height, const std::string_view& windowTitle) {
      application = std::make_unique<Application>(width, height, windowTitle);
   }

   void GameContainer::run() {
   }

   std::string GameContainer::createMesh(const std::string_view& filename) {
      return application->getResourceFactory()->createMesh(filename);
   }

   uint32_t GameContainer::createTexture(const std::string_view& filename) {
      return application->getResourceFactory()->createTexture(filename);
   }
}