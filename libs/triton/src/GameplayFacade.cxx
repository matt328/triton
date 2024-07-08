#include "tr/GameplayFacade.hpp"

#include "cm/Handles.hpp"

#include "gp/GameplaySystem.hpp"
#include "gfx/RenderContext.hpp"

namespace tr::ctx {
   GameplayFacade::GameplayFacade(gp::GameplaySystem& gameplaySystem,
                                  gfx::RenderContext& renderer,
                                  bool debugEnabled)
       : debugEnabled{debugEnabled}, gameplaySystem{gameplaySystem}, renderer{renderer} {
   }

   GameplayFacade::~GameplayFacade() {
   }

   // auto GameplayFacade::createTerrainEntity(const cm::MeshHandles handles) -> cm::EntityType {
   //    return gameplaySystem.entitySystem->createTerrain(handles);
   // }

   /*
      TODO: Consider having the ResourceManager not expose the std::future, but take in a callback
      function that will get called on the main thread when async loading has completed
      That would clean up this Facade significantly
      Enable the API in the commented out code below, and do away with the GameplayFacade::update()
      method

      Actually with the EntitySystem becoming thread safe, this can be greatly simplified. Can just
      load resources and then add them to the ES in the same 'Task'
      Actually actually maybe not since the resource manager is inside the renderer and doesn't know
      what clients of the resourcemanager might decide to do with the handles it returns.
      May be able to leverage futures to be able to specify continuations so that we can have
      modular tasks whose pieces can be decoupled from each other.
   */
   auto GameplayFacade::createTerrainMesh([[maybe_unused]] const uint32_t size)
       -> std::future<cm::ModelHandle> {

      // const auto callback = [this](cm::ModelHandle handle) {
      //    gameplaySystem.createTerrain(handle);
      // };

      // renderer.getResourceManager().createTerrain(callback);

      return renderer.getResourceManager().createTerrain();
   }

   std::future<cm::ModelHandle> GameplayFacade::loadModelAsync(const std::filesystem::path& path) {
      return renderer.getResourceManager().loadModelAsync(path.string());
   }

   void GameplayFacade::loadModelResources(
       [[maybe_unused]] const std::filesystem::path& modelPath,
       [[maybe_unused]] const std::filesystem::path& skeletonPath,
       [[maybe_unused]] const std::filesystem::path& animationPath,
       [[maybe_unused]] const std::function<void()> done) {
   }

   void GameplayFacade::update() {
      for (auto it = terrainFutures.begin(); it != terrainFutures.end();) {
         auto status = it->wait_for(std::chrono::seconds(0));
         if (status == std::future_status::ready) {
            ZoneNamedN(loadComplete, "Creating Terrain Entities", true);
            try {
               auto r = it->get();
               gameplaySystem.createTerrain(r);
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
       -> std::future<cm::LoadedSkinnedModelData> {
      return renderer.getResourceManager().loadSkinnedModelAsync(modelPath,
                                                                 skeletonPath,
                                                                 animationPath);
   }

   cm::EntityType GameplayFacade::createStaticMultiMeshEntity(cm::MeshHandles meshes) {
      return gameplaySystem.entitySystem->createStaticModel(meshes);
   }

   auto GameplayFacade::createSkinnedModelEntity(
       [[maybe_unused]] const cm::LoadedSkinnedModelData model) -> cm::EntityType {
      return gameplaySystem.entitySystem->createAnimatedModel(model);
   }

   cm::EntityType GameplayFacade::createCamera(uint32_t width,
                                               uint32_t height,
                                               float fov,
                                               float zNear,
                                               float zFar,
                                               glm::vec3 position,
                                               std::optional<std::string> name) {

      return gameplaySystem.entitySystem
          ->createCamera(width, height, fov, zNear, zFar, position, name);
   }

   void GameplayFacade::setCurrentCamera(cm::EntityType currentCamera) {
      return gameplaySystem.entitySystem->setCurrentCamera(currentCamera);
   }

   void GameplayFacade::clear() {
      gameplaySystem.entitySystem->removeAll();
   }

   auto GameplayFacade::getAnimationTimeRange(const cm::AnimationHandle handle)
       -> std::tuple<float, float> {
      const auto& animation = gameplaySystem.getAnimationFactory().getAnimation(handle);

      return std::make_tuple(0.f, animation.duration());
   }
}