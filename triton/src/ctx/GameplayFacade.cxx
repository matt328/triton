#include "ctx/GameplayFacade.hpp"

#include "gfx/Handles.hpp"
#include "gp/GameplaySystem.hpp"
#include "gfx/RenderContext.hpp"

#include "gp/ecs/component/Animation.hpp"
#include "gp/ecs/component/DebugConstants.hpp"
#include "gp/ecs/component/Renderable.hpp"
#include "gp/ecs/component/Terrain.hpp"
#include "gp/ecs/component/Transform.hpp"
#include "gp/ecs/component/Camera.hpp"
#include "gp/ecs/component/Resources.hpp"
#include "gp/ecs/component/DebugConstants.hpp"

namespace tr::ctx {
   GameplayFacade::GameplayFacade(gp::GameplaySystem& gameplaySystem,
                                  gfx::RenderContext& renderer,
                                  bool debugEnabled)
       : debugEnabled{debugEnabled}, gameplaySystem{gameplaySystem}, renderer{renderer} {
   }

   GameplayFacade::~GameplayFacade() {
   }

   auto GameplayFacade::createTerrainEntity(const MeshHandles handles) -> gp::EntityType {
      auto e = gameplaySystem.registry->create();

      gameplaySystem.registry->emplace<gp::ecs::Renderable>(e, handles);
      gameplaySystem.registry->emplace<gp::ecs::TerrainMarker>(e);
      gameplaySystem.registry->emplace<gp::ecs::Transform>(e,
                                                           glm::zero<glm::vec3>(),
                                                           glm::vec3(-550.f, -1000.f, -5700.f));

      if (debugEnabled) {
         gameplaySystem.registry->emplace<EditorInfoComponent>(e, "Terrain");
      }

      auto debugConstants = gameplaySystem.registry->create();
      gameplaySystem.registry->emplace<gp::ecs::Transform>(debugConstants,
                                                           glm::zero<glm::vec3>(),
                                                           glm::vec3(200.f, 1000.f, 200.f));
      gameplaySystem.registry->emplace<gp::ecs::DebugConstants>(debugConstants, 16.f);
      if (debugEnabled) {
         gameplaySystem.registry->emplace<EditorInfoComponent>(debugConstants, "PushConstants");
      }
      return e;
   }

   auto GameplayFacade::createTerrainMesh([[maybe_unused]] const uint32_t size)
       -> std::future<gfx::ModelHandle> {
      return renderer.getResourceManager().createTerrain();
   }

   std::future<gfx::ModelHandle> GameplayFacade::loadModelAsync(const std::filesystem::path& path) {
      return renderer.getResourceManager().loadModelAsync(path.string());
   }

   auto GameplayFacade::loadSkinnedModelAsync(const std::filesystem::path& modelPath,
                                              const std::filesystem::path& skeletonPath,
                                              const std::filesystem::path& animationPath)
       -> std::future<gfx::LoadedSkinnedModelData> {
      return renderer.getResourceManager().loadSkinnedModelAsync(modelPath,
                                                                 skeletonPath,
                                                                 animationPath);
   }

   gp::EntityType GameplayFacade::createStaticMultiMeshEntity(MeshHandles meshes) {
      auto e = gameplaySystem.registry->create();
      gameplaySystem.registry->emplace<gp::ecs::Renderable>(e, meshes);
      gameplaySystem.registry->emplace<gp::ecs::Transform>(e);

      if (debugEnabled) {
         gameplaySystem.registry->emplace<EditorInfoComponent>(e, "gltf model");
      }

      return e;
   }

   auto GameplayFacade::createSkinnedModelEntity(
       [[maybe_unused]] const gfx::LoadedSkinnedModelData model) -> gp::EntityType {
      auto e = gameplaySystem.registry->create();

      auto numJoints = this->renderer.getResourceManager()
                           .getAnimationFactory()
                           .getSkeleton(model.skeletonHandle)
                           .num_joints();
      auto numSoaJoints = this->renderer.getResourceManager()
                              .getAnimationFactory()
                              .getSkeleton(model.skeletonHandle)
                              .num_soa_joints();

      Log::debug << "numJoints: " << numJoints << std::endl;

      Log::debug << "numSoaJoints: " << numSoaJoints << std::endl;

      gameplaySystem.registry->emplace<gp::ecs::Animation>(e,
                                                           model.animationHandle,
                                                           model.skeletonHandle,
                                                           numJoints,
                                                           numSoaJoints);
      gameplaySystem.registry->emplace<gp::ecs::Transform>(e);

      const auto meshes = std::unordered_map<gfx::MeshHandle, gfx::TextureHandle>{
          {model.meshHandle, model.textureHandle}};

      gameplaySystem.registry->emplace<gp::ecs::Renderable>(e, meshes);

      if (debugEnabled) {
         gameplaySystem.registry->emplace<EditorInfoComponent>(e, "skinned model");
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
      gameplaySystem.registry->ctx().insert_or_assign<gp::ecs::CurrentCamera>(
          gp::ecs::CurrentCamera{currentCamera});
   }

   void GameplayFacade::clear() {
      gameplaySystem.registry->clear();
      gameplaySystem.registry->ctx().erase<gp::ecs::CurrentCamera>();
   }

   std::string& GameplayFacade::getActiveCameraName() {
      auto c = gameplaySystem.registry->ctx().get<gp::ecs::CurrentCamera>();
      auto e = getComponent<EditorInfoComponent>(c.currentCamera);
      return e.value().get().name;
   }

   std::vector<gp::EntityType>& GameplayFacade::getAllEntities() {
      allEntities.clear();
      for (auto e : gameplaySystem.registry->view<entt::entity>()) {
         allEntities.push_back(e);
      }
      return allEntities;
   }

   OptionalRef<gp::ecs::Transform> GameplayFacade::getEntityTransform(gp::EntityType entityId) {
      if (gameplaySystem.registry->all_of<gp::ecs::Transform>(entityId)) {
         auto& v = gameplaySystem.registry->get<gp::ecs::Transform>(entityId);
         return std::optional<std::reference_wrapper<gp::ecs::Transform>>{std::ref(v)};
      }
      return {};
   }

   EditorInfoComponent& GameplayFacade::getEditorInfo(gp::EntityType entityId) {
      auto& nameComponent = gameplaySystem.registry->get<EditorInfoComponent>(entityId);
      return nameComponent;
   }

   OptionalRef<gp::ecs::Camera> GameplayFacade::getCameraComponent(const gp::EntityType entityId) {
      if (gameplaySystem.registry->all_of<gp::ecs::Camera>(entityId)) {
         auto& c = gameplaySystem.registry->get<gp::ecs::Camera>(entityId);
         return std::optional<std::reference_wrapper<gp::ecs::Camera>>{std::ref(c)};
      }
      return {};
   }
}