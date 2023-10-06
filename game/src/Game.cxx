#include <GL/glext.h>
#include <utility>

#include "ActionManager.hpp"
#include "ActionType.hpp"
#include "Events.hpp"
#include "Game.hpp"

#include "ResourceFactory.hpp"
#include "components/Renderable.hpp"
#include "components/Transform.hpp"
#include "components/Camera.hpp"

#include "systems/RenderSystem.hpp"
#include "systems/TransformSystem.hpp"
#include "systems/InputSystem.hpp"
#include "Paths.hpp"
#include "Logger.hpp"
#include "systems/CameraController.hpp"
#include "InputMapper.hpp"

namespace game {
   class Game::GameImpl {
    public:
      GameImpl(IResourceFactory* factory, int width, int height) : resourceFactory(factory) {
         registry = std::make_unique<entt::registry>();

         actionManager = std::make_unique<Input::ActionManager>();
         inputMapper = std::make_unique<Input::InputMapper>(*actionManager);
         actionManager->mapKey(Input::Key::Up, Input::ActionType::MoveForward);
         actionManager->mapKey(Input::Key::W, Input::ActionType::MoveForward);

         renderSystem = std::make_shared<RenderSystem>();
         transformSystem = std::make_unique<TransformSystem>();
         inputSystem = std::make_unique<InputSystem>();

         // Create viking room entity
         const auto textureFilename = (Core::Paths::TEXTURES / "viking_room.png").string();
         const auto filename = (Core::Paths::MODELS / "viking_room.gltf").string();

         const auto meshId = resourceFactory->createMesh(filename);
         const auto textureId = resourceFactory->createTexture(textureFilename);

         const auto room = registry->create();
         registry->emplace<Renderable>(room, meshId, textureId);
         registry->emplace<Transform>(room);

         // Create Plane area entity
         const auto planeMeshId =
             resourceFactory->createMesh((Core::Paths::MODELS / "area.gltf").string());
         const auto planeTextureId =
             resourceFactory->createTexture((Core::Paths::TEXTURES / "grass.png").string());

         const auto floor = registry->create();
         registry->emplace<Renderable>(floor, planeMeshId, planeTextureId);
         registry->emplace<Transform>(floor);

         // Create Camera entity
         const auto camera = registry->create();
         // NOLINTBEGIN TODO move these somewhere else
         registry->emplace<Camera>(
             camera, 60.f, width, height, 0.1f, 1000.f, glm::vec3(2.f, 2.f, 2.f));
         // NOLINTEND
         registry->emplace<Transform>(camera);

         auto cameraController = std::make_unique<CameraController>();

         // cameraController->setActiveCamera(camera);

         // Bind up all the camera controller's actions it knows how to handle
         actionManager->onAction(
             Input::ActionType::MoveForward,
             [&cameraController](Input::Action a) -> void { cameraController->moveForward(a); });
      }

      void onEvent(Events::Event& e) {
         auto dispatcher = Events::EventDispatcher{e};

         dispatcher.dispatch<Events::EventType::KeyPressed>(std)
      }

      void keyEvent(Events::&kpe) {
      }

      void keyPressed(const int key, int scancode, const int action, int mods) const {
         inputMapper->keyCallback(key, scancode, action, mods);
      }
      void update() const {
         transformSystem->update(*registry);
         renderSystem->update(*registry);
         // if (inputSystem->isActionActive(Actions::Action::MoveForward)) {
         //    Log::info << "Move Forward Active" << std::endl;
         // };
      }

      void blendState([[maybe_unused]] double alpha) {
         // state = currentState * alpha + previousState * (1.0 - alpha);
         // Log::core->debug("blendState(), alpha: {}", alpha);
      }

      std::vector<RenderObject> getRenderObjects() {
         return renderSystem->getRenderObjects();
      }

      std::tuple<glm::mat4, glm::mat4, glm::mat4> getCameraParams() {
         return renderSystem->getCameraParams();
      }

    private:
      entt::delegate<void(int, int, int, int)> keyDelegate;
      IResourceFactory* resourceFactory;
      std::unique_ptr<entt::registry> registry;
      std::shared_ptr<RenderSystem> renderSystem;
      std::unique_ptr<TransformSystem> transformSystem;
      std::unique_ptr<InputSystem> inputSystem;
      std::unique_ptr<Input::ActionManager> actionManager;
      std::unique_ptr<Input::InputMapper> inputMapper;
   };

   Game::Game(IResourceFactory* factory, int width, int height) : IGame() {
      impl = std::make_unique<GameImpl>(factory, width, height);
   }

   Game::~Game() = default;

   void Game::update() {
      impl->update();
   }

   void Game::blendState(double alpha) {
      impl->blendState(alpha);
   }

   void Game::keyPressed(const int key, int scancode, const int action, int mods) {
      impl->keyPressed(key, scancode, action, mods);
   }

   std::vector<RenderObject> Game::getRenderObjects() {
      return impl->getRenderObjects();
   }

   std::tuple<glm::mat4, glm::mat4, glm::mat4> Game::getCameraParams() {
      return impl->getCameraParams();
   }

   void Game::onEvent(Events::Event& e) {
      impl->onEvent(e);
   }

};
