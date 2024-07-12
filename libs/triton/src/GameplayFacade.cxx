#include "tr/GameplayFacade.hpp"

#include "cm/Handles.hpp"

#include "gp/GameplaySystem.hpp"
#include "gfx/RenderContext.hpp"

namespace tr::ctx {

   class GameplayFacade::GameplayFacade::Impl {
    public:
      Impl(gp::GameplaySystem& gameplaySystem, gfx::RenderContext& renderer, bool debugEnabled)
          : debugEnabled{debugEnabled}, gameplaySystem{gameplaySystem}, renderer{renderer} {
      }

      auto createTerrainEntity(const cm::MeshHandles handles) -> cm::EntityType {
         return gameplaySystem.createTerrain(handles);
      }

      auto createTerrainMesh([[maybe_unused]] const uint32_t size) {

         /*
            Have these methods also take in an optional lambda to add to the then chain
            so that the UI of the editor can know when they've completed
            Also whatever logic system caused them to be loaded in a game might also want to
            register a callback for when it's been loaded. This logic system will be encapsulated in
            a 'Game' module. The editor will export a set of game data files that a single Game
            module will know how to start up and run. The Game module will load up the assets and
            scripts and stuff that the editor produces and be able to just run.
            TODO: see if entt's events can handle this?
         */

         // const auto createTerrainEntity = [this](cm::ModelHandle handle) {
         //    gameplaySystem.createTerrain(handle);
         // };
         // renderer.getResourceManager().createTerrain().then(createTerrainEntity);

         return renderer.createTerrain();
      }

      auto loadModelAsync(const std::filesystem::path& path) {
         return renderer.loadModelAsync(path.string());
      }

      void loadModelResources([[maybe_unused]] const std::filesystem::path& modelPath,
                              [[maybe_unused]] const std::filesystem::path& skeletonPath,
                              [[maybe_unused]] const std::filesystem::path& animationPath,
                              [[maybe_unused]] const std::function<void()> done) {
      }

      void update() {
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

      auto loadSkinnedModelAsync(const std::filesystem::path& modelPath,
                                 const std::filesystem::path& skeletonPath,
                                 const std::filesystem::path& animationPath) {
         return renderer.loadSkinnedModelAsync(modelPath, skeletonPath, animationPath);
      }

      auto createStaticMultiMeshEntity(cm::MeshHandles meshes) {
         return gameplaySystem.createStaticModel(meshes);
      }

      auto createSkinnedModelEntity(const cm::LoadedSkinnedModelData model) {
         return gameplaySystem.createAnimatedModel(model);
      }

      auto createCamera(uint32_t width,
                        uint32_t height,
                        float fov,
                        float zNear,
                        float zFar,
                        glm::vec3 position,
                        std::optional<std::string> name) {

         return gameplaySystem.createCamera(width, height, fov, zNear, zFar, position, name);
      }

      void setCurrentCamera(cm::EntityType currentCamera) {
         gameplaySystem.setCurrentCamera(currentCamera);
      }

      void clear() {
         gameplaySystem.clearEntities();
      }

    private:
      bool debugEnabled{};
      gp::GameplaySystem& gameplaySystem;
      gfx::RenderContext& renderer;
      std::vector<cm::EntityType> allEntities;

      std::vector<std::future<tr::cm::ModelHandle>> terrainFutures{};
   };

   GameplayFacade::GameplayFacade(gp::GameplaySystem& gameplaySystem,
                                  gfx::RenderContext& renderer,
                                  bool debugEnabled)
       : impl(std::make_unique<Impl>(gameplaySystem, renderer, debugEnabled)) {
   }

   GameplayFacade::~GameplayFacade() {
   }

   auto GameplayFacade::createTerrainEntity(const cm::MeshHandles handles) -> cm::EntityType {
      return impl->createTerrainEntity(handles);
   }

   auto GameplayFacade::createTerrainMesh([[maybe_unused]] const uint32_t size)
       -> std::future<cm::ModelHandle> {

      /*
         Have these methods also take in an optional lambda to add to the then chain
         so that the UI of the editor can know when they've completed
         Also whatever logic system caused them to be loaded in a game might also want to register
         a callback for when it's been loaded.
         This logic system will be encapsulated in a 'Game' module.
         The editor will export a set of game data files that a single Game module will know how to
         start up and run. The Game module will load up the assets and scripts and stuff that the
         editor produces and be able to just run.
         TODO: see if entt's events can handle this?
      */

      // const auto createTerrainEntity = [this](cm::ModelHandle handle) {
      //    gameplaySystem.createTerrain(handle);
      // };
      // renderer.getResourceManager().createTerrain().then(createTerrainEntity);

      return impl->createTerrainMesh(size);
   }

   auto GameplayFacade::loadModelAsync(const std::filesystem::path& path)
       -> std::future<cm::ModelHandle> {
      return impl->loadModelAsync(path);
   }

   void GameplayFacade::loadModelResources(
       [[maybe_unused]] const std::filesystem::path& modelPath,
       [[maybe_unused]] const std::filesystem::path& skeletonPath,
       [[maybe_unused]] const std::filesystem::path& animationPath,
       [[maybe_unused]] const std::function<void()> done) {
   }

   void GameplayFacade::update() {
      impl->update();
   }

   auto GameplayFacade::loadSkinnedModelAsync(const std::filesystem::path& modelPath,
                                              const std::filesystem::path& skeletonPath,
                                              const std::filesystem::path& animationPath)
       -> std::future<cm::LoadedSkinnedModelData> {
      return impl->loadSkinnedModelAsync(modelPath, skeletonPath, animationPath);
   }

   cm::EntityType GameplayFacade::createStaticMultiMeshEntity(cm::MeshHandles meshes) {
      return impl->createStaticMultiMeshEntity(meshes);
   }

   auto GameplayFacade::createSkinnedModelEntity(const cm::LoadedSkinnedModelData model)
       -> cm::EntityType {
      return impl->createSkinnedModelEntity(model);
   }

   cm::EntityType GameplayFacade::createCamera(uint32_t width,
                                               uint32_t height,
                                               float fov,
                                               float zNear,
                                               float zFar,
                                               glm::vec3 position,
                                               std::optional<std::string> name) {

      return impl->createCamera(width, height, fov, zNear, zFar, position, name);
   }

   void GameplayFacade::setCurrentCamera(cm::EntityType currentCamera) {
      return impl->setCurrentCamera(currentCamera);
   }

   void GameplayFacade::clear() {
      impl->clear();
   }

}