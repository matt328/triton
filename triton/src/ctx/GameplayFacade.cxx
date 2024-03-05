#include "ctx/GameplayFacade.hpp"

#include "gp/GameplaySystem.hpp"
#include "gfx/Renderer.hpp"

#include "gp/ecs/component/Renderable.hpp"
#include "gp/ecs/component/Transform.hpp"
#include "gp/ecs/component/Camera.hpp"
#include "gp/ecs/component/Resources.hpp"
#include <functional>
#include <optional>

namespace tr::ctx {
   GameplayFacade::GameplayFacade(gp::GameplaySystem& gameplaySystem,
                                  gfx::Renderer& renderer,
                                  bool debugEnabled)
       : debugEnabled{debugEnabled}, gameplaySystem{gameplaySystem}, renderer{renderer} {
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

      if (debugEnabled && name.has_value()) {
         gameplaySystem.registry->emplace<EditorInfoComponent>(e,
                                                               name.value(),
                                                               meshFile,
                                                               textureFile);
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

      if (debugEnabled && name.has_value()) {
         gameplaySystem.registry->emplace<EditorInfoComponent>(camera, name.value());
      }
      return camera;
   }

   void GameplayFacade::setCurrentCamera(gp::EntityType currentCamera) {
      gameplaySystem.registry->ctx().emplace<gp::ecs::CurrentCamera>(currentCamera);
   }

   // TODO: Get ALL of the entities not just non camera ones.
   std::vector<gp::EntityType>& GameplayFacade::getAllEntities() {
      allEntities.clear();
      for (auto e : gameplaySystem.registry->view<gp::ecs::Renderable, gp::ecs::Transform>()) {
         allEntities.push_back(e);
      }
      return allEntities;
   }

   gp::ecs::Transform& GameplayFacade::getEntityTransform(gp::EntityType entityId) {
      auto& v = gameplaySystem.registry->get<gp::ecs::Transform>(entityId);
      return v;
   }

   EditorInfoComponent& GameplayFacade::getEditorInfo(gp::EntityType entityId) {
      auto& nameComponent = gameplaySystem.registry->get<EditorInfoComponent>(entityId);
      return nameComponent;
   }

   std::optional<std::reference_wrapper<gp::ecs::Camera>> GameplayFacade::getCameraComponent(
       const gp::EntityType entityId) {
      if (gameplaySystem.registry->all_of<gp::ecs::Camera>(entityId)) {
         auto& c = gameplaySystem.registry->get<gp::ecs::Camera>(entityId);
         return std::optional<std::reference_wrapper<gp::ecs::Camera>>{std::ref(c)};
      }
      return {};
   }
}