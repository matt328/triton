#include "tr/GameplayFacade.hpp"

#include "cm/EntitySystemTypes.hpp"
#include "cm/Handles.hpp"

#include "gfx/RenderContext.hpp"

#include "gp/GameplaySystem.hpp"

#include "BaseException.hpp"

namespace tr::ctx {

   class GameplayFacade::Impl {
    public:
      Impl(gp::GameplaySystem& gameplaySystem, gfx::RenderContext& renderer)
          : gameplaySystem{gameplaySystem}, renderer{renderer} {
      }

      [[nodiscard]] auto getCreateTerrainFn() const -> std::function<cm::EntityType(uint32_t)> {
         return [this](uint32_t size) -> cm::EntityType {
            auto terrainResult = renderer.createTerrain(size);
            return gameplaySystem.createTerrain(terrainResult);
         };
      }

      [[nodiscard]] auto createStaticModelEntity(
          const std::filesystem::path& modelPath) const noexcept -> cm::EntityType {
         auto staticModelResult = renderer.createStaticModel(modelPath);
         return gameplaySystem.createStaticModel(staticModelResult);
      }

      [[nodiscard]] auto getAnimatedModelEntityTask() const
          -> std::function<cm::EntityType(const std::filesystem::path&,
                                          const std::filesystem::path&,
                                          const std::filesystem::path&)> {
         return [this](const std::filesystem::path& modelPath,
                       const std::filesystem::path& skeletonPath,
                       const std::filesystem::path& animationPath) -> cm::EntityType {
            try {
               auto modelResult = renderer.createSkinnedModel(modelPath);
               return gameplaySystem.createAnimatedModel(modelResult, skeletonPath, animationPath);
            } catch (tr::BaseException& ex) {
               ex << "CreateAnimatedModelEntityTask: ";
               throw;
            }
         };
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

   auto GameplayFacade::getCreateTerrainFn() const -> std::function<cm::EntityType(uint32_t)> {
      return impl->getCreateTerrainFn();
   }

   [[nodiscard]] auto GameplayFacade::createStaticModelEntity(
       const std::filesystem::path& modelPath) const noexcept -> cm::EntityType {
      return impl->createStaticModelEntity(modelPath);
   }

   [[nodiscard]] auto GameplayFacade::getAnimatedModelEntityTask() const
       -> std::function<cm::EntityType(const std::filesystem::path&,
                                       const std::filesystem::path&,
                                       const std::filesystem::path&)> {
      return impl->getAnimatedModelEntityTask();
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

} // namespace tr::ctx