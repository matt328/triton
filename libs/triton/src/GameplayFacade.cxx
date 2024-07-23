#include "tr/GameplayFacade.hpp"

#include "cm/Handles.hpp"

#include "gp/GameplaySystem.hpp"
#include "gfx/RenderContext.hpp"

namespace tr::ctx {

   class GameplayFacade::GameplayFacade::Impl {
    public:
      Impl(gp::GameplaySystem& gameplaySystem, gfx::RenderContext& renderer)
          : gameplaySystem{gameplaySystem}, renderer{renderer} {
      }

      auto createTerrain(const uint32_t size) {
         ZoneNamedN(n, "facade.createTerrain", true);
         const auto createEntity = [this](cm::ModelData handle) {
            return gameplaySystem.createTerrain(handle);
         };
         return renderer.createTerrain(size).then(createEntity);
      }

      auto createStaticModelEntity(const std::filesystem::path& modelPath)
          -> futures::cfuture<cm::EntityType> {

         auto gpCreate = [this](cm::ModelData handles) {
            return gameplaySystem.createStaticModel(handles);
         };

         auto createModel = renderer.createStaticModel(modelPath);

         return createModel.then(gpCreate);
      }

      auto createAnimatedModelEntity(const std::filesystem::path& modelPath,
                                     const std::filesystem::path& skeletonPath,
                                     const std::filesystem::path& animationPath)
          -> futures::cfuture<cm::EntityType> {

         auto gpCreate = [this, &skeletonPath, &animationPath](cm::ModelData modelData) {
            return gameplaySystem.createAnimatedModel(modelData, skeletonPath, animationPath);
         };

         auto createModel = renderer.createSkinnedModel(modelPath);

         return createModel.then(gpCreate);
      }

      void loadModelResources([[maybe_unused]] const std::filesystem::path& modelPath,
                              [[maybe_unused]] const std::filesystem::path& skeletonPath,
                              [[maybe_unused]] const std::filesystem::path& animationPath,
                              [[maybe_unused]] const std::function<void()> done) {
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
      gp::GameplaySystem& gameplaySystem;
      gfx::RenderContext& renderer;
      std::vector<cm::EntityType> allEntities;
   };

   GameplayFacade::GameplayFacade(gp::GameplaySystem& gameplaySystem, gfx::RenderContext& renderer)
       : impl(std::make_unique<Impl>(gameplaySystem, renderer)) {
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