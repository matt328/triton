#include "ctx/GameplayFacade.hpp"

#include "gfx/Handles.hpp"
#include "gp/GameplaySystem.hpp"
#include "gfx/RenderContext.hpp"

#include "gp/ecs/component/Animation.hpp"
#include "gp/ecs/component/Renderable.hpp"
#include "gp/ecs/component/Transform.hpp"
#include "gp/ecs/component/Camera.hpp"
#include "gp/ecs/component/Resources.hpp"
#include "gp/EntitySystem.hpp"

namespace tr::ctx {
   GameplayFacade::GameplayFacade(gp::GameplaySystem& gameplaySystem,
                                  gfx::RenderContext& renderer,
                                  bool debugEnabled)
       : debugEnabled{debugEnabled}, gameplaySystem{gameplaySystem}, renderer{renderer} {
   }

   GameplayFacade::~GameplayFacade() {
   }

   auto GameplayFacade::createTerrainEntity(const gfx::MeshHandles handles) -> gp::EntityType {
      return gameplaySystem.entitySystem->createTerrain(handles);
   }

   auto GameplayFacade::createTerrainMesh([[maybe_unused]] const uint32_t size)
       -> std::future<gfx::ModelHandle> {
      return renderer.getResourceManager().createTerrain();
   }

   std::future<gfx::ModelHandle> GameplayFacade::loadModelAsync(const std::filesystem::path& path) {
      return renderer.getResourceManager().loadModelAsync(path.string());
   }

   void GameplayFacade::loadModelResources(const std::filesystem::path& modelPath,
                                           const std::filesystem::path& skeletonPath,
                                           const std::filesystem::path& animationPath,
                                           const std::function<void()> done) {
   }

   void GameplayFacade::update() {
      for (auto it = terrainFutures.begin(); it != terrainFutures.end();) {
         auto status = it->wait_for(std::chrono::seconds(0));
         if (status == std::future_status::ready) {
            ZoneNamedN(loadComplete, "Creating Terrain Entities", true);
            try {
               auto r = it->get();
               createTerrainEntity(r);
            } catch (const std::exception& e) {
               Log::error << "error loading model: " << e.what() << std::endl;
            }
            it = terrainFutures.erase(it);
         } else {
            ++it;
         }
      }
   }

   auto GameplayFacade::loadSkinnedModelAsync(const std::filesystem::path& modelPath,
                                              const std::filesystem::path& skeletonPath,
                                              const std::filesystem::path& animationPath)
       -> std::future<gfx::LoadedSkinnedModelData> {
      return renderer.getResourceManager().loadSkinnedModelAsync(modelPath,
                                                                 skeletonPath,
                                                                 animationPath);
   }

   gp::EntityType GameplayFacade::createStaticMultiMeshEntity(gfx::MeshHandles meshes) {
      return gameplaySystem.entitySystem->createStaticModel(meshes);
   }

   auto GameplayFacade::createSkinnedModelEntity(
       [[maybe_unused]] const gfx::LoadedSkinnedModelData model) -> gp::EntityType {
      return gameplaySystem.entitySystem->createAnimatedModel(model);
   }

   gp::EntityType GameplayFacade::createCamera(uint32_t width,
                                               uint32_t height,
                                               float fov,
                                               float zNear,
                                               float zFar,
                                               glm::vec3 position,
                                               std::optional<std::string> name) {

      return gameplaySystem.entitySystem
          ->createCamera(width, height, fov, zNear, zFar, position, name);
   }

   void GameplayFacade::setCurrentCamera(gp::EntityType currentCamera) {
      return gameplaySystem.entitySystem->setCurrentCamera(currentCamera);
   }

   void GameplayFacade::clear() {
      gameplaySystem.entitySystem->removeAll();
   }

   auto GameplayFacade::getAnimationTimeRange(const gfx::AnimationHandle handle)
       -> std::tuple<float, float> {
      const auto& animation = gameplaySystem.getAnimationFactory().getAnimation(handle);

      return std::make_tuple(0.f, animation.duration());
   }
}