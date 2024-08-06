#include "tr/GameplayFacade.hpp"

#include "cm/Handles.hpp"

#include "gp/GameplaySystem.hpp"
#include "gfx/RenderContext.hpp"

#include "BaseException.hpp"

namespace tr::ctx {

   class GameplayFacade::Impl {
    public:
      Impl(gp::GameplaySystem& gameplaySystem, gfx::RenderContext& renderer)
          : gameplaySystem{gameplaySystem}, renderer{renderer} {
      }

      [[nodiscard]] auto createTerrain(const uint32_t size) const {
         ZoneNamedN(n, "facade.createTerrain", true);
         const auto createEntity = [this](const cm::ModelData& handle) {
            return gameplaySystem.createTerrain(handle);
         };
         return renderer.createTerrain(size).then(createEntity);
      }

      [[nodiscard]] auto createStaticModelEntity(const std::filesystem::path& modelPath) const noexcept
          -> futures::cfuture<cm::EntityType> {

         auto gpCreate = [this](const cm::ModelData& handles) {
            return gameplaySystem.createStaticModel(handles);
         };

         auto createModel = renderer.createStaticModel(modelPath);
         return createModel.then(gpCreate);
      }

      [[nodiscard]] auto createAnimatedModelEntity(const std::filesystem::path& modelPath,
                                                   const std::filesystem::path& skeletonPath,
                                                   const std::filesystem::path& animationPath) const
          -> futures::cfuture<cm::EntityType> {

         auto gpCreate = [this, skeletonPath, animationPath](const cm::ModelData& modelData) {
            return gameplaySystem.createAnimatedModel(modelData, skeletonPath, animationPath);
         };

         try {
            auto createModel = renderer.createSkinnedModel(modelPath);
            return createModel.then(gpCreate);
         } catch (tr::BaseException& ex) {
            ex << "GameplayFacade::createAnimatedModelEntity(): ";
            throw;
         }
      }

      static void loadModelResources([[maybe_unused]] const std::filesystem::path& modelPath,
                                     [[maybe_unused]] const std::filesystem::path& skeletonPath,
                                     [[maybe_unused]] const std::filesystem::path& animationPath,
                                     [[maybe_unused]] const std::function<void()>& done) {
      }

      [[nodiscard]] auto createCamera(const uint32_t width,
                                      const uint32_t height,
                                      const float fov,
                                      const float zNear,
                                      const float zFar,
                                      const glm::vec3& position,
                                      const std::optional<std::string>& name) const {

         return gameplaySystem.createCamera(width, height, fov, zNear, zFar, position, name);
      }

      void setCurrentCamera(const cm::EntityType currentCamera) const {
         gameplaySystem.setCurrentCamera(currentCamera);
      }

      void clear() const {
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

   GameplayFacade::~GameplayFacade() { // NOLINT(*-use-equals-default)
   }

   auto GameplayFacade::createStaticModelEntity(const std::filesystem::path& modelPath) const noexcept
       -> futures::cfuture<cm::EntityType> {
      return impl->createStaticModelEntity(modelPath);
   }

   auto GameplayFacade::createAnimatedModelEntity(const std::filesystem::path& modelPath,
                                                  const std::filesystem::path& skeletonPath,
                                                  const std::filesystem::path& animationPath) const
       -> futures::cfuture<cm::EntityType> {
      return impl->createAnimatedModelEntity(modelPath, skeletonPath, animationPath);
   }

   auto GameplayFacade::createTerrain(const uint32_t size) const
       -> futures::cfuture<cm::EntityType> {
      return impl->createTerrain(size);
   }

   cm::EntityType GameplayFacade::createCamera(const uint32_t width,
                                               const uint32_t height,
                                               const float fov,
                                               const float zNear,
                                               const float zFar,
                                               const glm::vec3& position,
                                               const std::optional<std::string>& name) const {

      return impl->createCamera(width, height, fov, zNear, zFar, position, name);
   }

   void GameplayFacade::setCurrentCamera(const cm::EntityType currentCamera) const {
      return impl->setCurrentCamera(currentCamera);
   }

   void GameplayFacade::clear() const {
      impl->clear();
   }

}