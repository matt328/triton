#include "ctx/GameplayFacade.hpp"

#include "gp/GameplaySystem.hpp"
#include "gfx/Renderer.hpp"

#include "gp/ecs/component/Renderable.hpp"
#include "gp/ecs/Transform.hpp"
#include "gp/ecs/component/Camera.hpp"
#include "gp/ecs/component/Resources.hpp"

namespace tr::ctx {
   GameplayFacade::GameplayFacade(gp::GameplaySystem& gameplaySystem, gfx::Renderer& renderer)
       : gameplaySystem{gameplaySystem}, renderer{renderer} {
   }

   GameplayFacade::~GameplayFacade() {
   }

   gp::EntityType GameplayFacade::createStaticMeshEntity(std::string meshFile,
                                                         std::string textureFile,
                                                         std::optional<std::string> name) {
      const auto meshId = renderer.createMesh(meshFile);
      const auto textureId = renderer.createTexture(textureFile);

      auto e = gameplaySystem.registry->create();
      gameplaySystem.registry->emplace<gp::ecs::Renderable>(e, meshId, textureId);
      gameplaySystem.registry->emplace<gp::ecs::Transform>(e);

      if (name.has_value()) {
         gameplaySystem.registry->emplace<NameComponent>(e, name.value());
      }

      return e;
   }

   gp::EntityType GameplayFacade::createCamera(uint32_t width,
                                               uint32_t height,
                                               float fov,
                                               float zNear,
                                               float zFar,
                                               glm::vec3 position,
                                               std::optional<std::string> name) {

      const auto camera = gameplaySystem.registry->create();
      gameplaySystem.registry
          ->emplace<gp::ecs::Camera>(camera, width, height, fov, zNear, zFar, position);

      if (name.has_value()) {
         gameplaySystem.registry->emplace<NameComponent>(camera, name.value());
      }
      return camera;
   }

   void GameplayFacade::setCurrentCamera(gp::EntityType currentCamera) {
      gameplaySystem.registry->ctx().emplace<gp::ecs::CurrentCamera>(currentCamera);
   }

   std::vector<gp::EntityType>& GameplayFacade::getAllEntities() {
      allEntities.clear();
      for (auto e : gameplaySystem.registry->view<gp::ecs::Renderable, gp::ecs::Transform>()) {
         allEntities.push_back(e);
      }
      return allEntities;
   }

   gp::ecs::Transform& GameplayFacade::getEntityPosition(gp::EntityType entityId) {
      auto& v = gameplaySystem.registry->get<gp::ecs::Transform>(entityId);
      return v;
   }

   NameComponent& GameplayFacade::getEntityName(gp::EntityType entityId) {
      auto& nameComponent = gameplaySystem.registry->get<NameComponent>(entityId);
      return nameComponent;
   }
}