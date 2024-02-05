#include "Game.hpp"

#include "core/Paths.hpp"
#include "graphics/RenderObject.hpp"
#include "graphics/Renderer.hpp"

namespace Triton::Game {
   // HACK: This entire class.  slopping stuff in here to manually test out the renderer before
   // adding proper ECS.
   Game::Game(GLFWwindow* window) {
      renderer = std::make_unique<Graphics::Renderer>(window);
      // Create viking room entity
      const auto textureFilename = (Core::Paths::TEXTURES / "viking_room.png").string();
      const auto filename = (Core::Paths::MODELS / "viking_room.gltf").string();

      const auto meshId = renderer->createMesh(filename);
      const auto textureId = renderer->createTexture(textureFilename);

      renderer->registerRenderObjectProvider([meshId, textureId] {
         const auto renderObject = Graphics::RenderObject{
             .meshId = meshId,
             .textureId = textureId,
             .modelMatrix = glm::identity<glm::mat4>(),
         };
         return std::vector<Graphics::RenderObject>{renderObject};
      });

      renderer->registerPerFrameDataProvider([] {
         const auto perFrameData = Graphics::PerFrameData{
             .view = glm::identity<glm::mat4>(),
             .proj = glm::identity<glm::mat4>(),
             .viewProj = glm::identity<glm::mat4>(),
         };
         return perFrameData;
      });
   }

   Game::~Game() {
      Log::info << "destroying game" << std::endl;
   };

   void Game::update(const Core::Timer& timer) {
   }

   void Game::render() {
      renderer->render();
   }

   void Game::resize(const int width, const int height) {
      renderer->windowResized(width, height);
   }

   void Game::waitIdle() {
      renderer->waitIdle();
   }
}