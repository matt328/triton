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

      auto createTerrain(const uint32_t size) {
         ZoneNamedN(n, "facade.createTerrain", true);
         const auto createEntity = [this](cm::ModelHandle handle) {
            return gameplaySystem.createTerrain(handle);
         };
         return renderer.createTerrain(size).then(createEntity);
      }

      auto createStaticModelEntity(const std::filesystem::path& modelPath)
          -> futures::cfuture<cm::EntityType> {

         const auto createEntity = [this](cm::MeshHandles handles) {
            return gameplaySystem.createStaticModel(handles);
         };
         return renderer.createStaticModel(modelPath).then(createEntity);
      }

      auto createAnimatedModelEntity(const std::filesystem::path& modelPath,
                                     const std::filesystem::path& skeletonPath,
                                     const std::filesystem::path& animationPath)
          -> futures::cfuture<cm::EntityType> {
         // I think here we should have the gameplay system load the skeleton and animation
         // Probably won't need anything different other than loadTrm from the renderer.
         // it can just return the meshhandles and the gameplay system will add in the skeleton and
         // animation handles
         const auto createEntity = [this](cm::MeshHandles handles) {
            return gameplaySystem.createAnimatedModel(handles, skeletonPath, animationPath);
         };

         return renderer.createStaticModel(modelPath).then(createEntity);
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
                  Log.error("Error loading model: {0}", e.what());
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

   auto GameplayFacade::createStaticModelEntity(const std::filesystem::path& modelPath)
       -> futures::cfuture<cm::EntityType> {
      return impl->createStaticModelEntity(modelPath);
   }

   auto GameplayFacade::createAnimatedModelEntity(const std::filesystem::path& modelPath,
                                                  const std::filesystem::path& skeletonPath,
                                                  const std::filesystem::path& animationPath)
       -> futures::cfuture<cm::EntityType> {
      return impl->createAnimatedModelEntity(modelPath, skeletonPath, animationPath);
   }

   auto GameplayFacade::createTerrain(const uint32_t size) -> futures::cfuture<cm::EntityType> {
      return impl->createTerrain(size);
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