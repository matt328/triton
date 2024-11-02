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

      void createTerrain() const {
         auto modelDatas = renderer.createTerrain();
         gameplaySystem.createTerrain(modelDatas);
      }

      [[nodiscard]] auto createDebugAABB(const glm::vec3& min, const glm::vec3& max) const noexcept
          -> cm::EntityType {
         const auto aabbHandle = renderer.createAABBGeometry(min, max);
         return gameplaySystem.createStaticModel(aabbHandle);
      }

      [[nodiscard]] auto createStaticModelEntity(
          const std::filesystem::path& modelPath) const noexcept -> cm::EntityType {
         auto modelData = renderer.createStaticModel(modelPath);
         return gameplaySystem.createStaticModel(modelData);
      }

      [[nodiscard]] auto createAnimatedModelEntity(const std::filesystem::path& modelPath,
                                                   const std::filesystem::path& skeletonPath,
                                                   const std::filesystem::path& animationPath) const
          -> cm::EntityType {
         try {
            auto modelResult = renderer.createSkinnedModel(modelPath);
            return gameplaySystem.createAnimatedModel(modelResult, skeletonPath, animationPath);
         } catch (tr::BaseException& ex) {
            ex << "CreateAnimatedModelEntityTask: ";
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

      void addTerrainCreatedListener(const cm::TerrainCreatedFn& func) {
         gameplaySystem.addTerrainCreatedListener(func);
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

   void GameplayFacade::createTerrain() const {
      impl->createTerrain();
   }

   [[nodiscard]] auto GameplayFacade::createDebugAABB(const glm::vec3& min,
                                                      const glm::vec3& max) const
       -> cm::EntityType {
      return impl->createDebugAABB(min, max);
   }

   [[nodiscard]] auto GameplayFacade::createStaticModelEntity(
       const std::filesystem::path& modelPath) const noexcept -> cm::EntityType {
      return impl->createStaticModelEntity(modelPath);
   }

   [[nodiscard]] auto GameplayFacade::createAnimatedModelEntity(
       const std::filesystem::path& modelPath,
       const std::filesystem::path& skeletonPath,
       const std::filesystem::path& animationPath) const -> cm::EntityType {
      return impl->createAnimatedModelEntity(modelPath, skeletonPath, animationPath);
   }

   auto GameplayFacade::createCamera(const uint32_t width,
                                     const uint32_t height,
                                     const float fov,
                                     const float zNear,
                                     const float zFar,
                                     const glm::vec3& position,
                                     const std::optional<std::string>& name) const
       -> cm::EntityType {

      return impl->createCamera(width, height, fov, zNear, zFar, position, name);
   }

   void GameplayFacade::setCurrentCamera(const cm::EntityType currentCamera) const {
      return impl->setCurrentCamera(currentCamera);
   }

   void GameplayFacade::clear() const {
      impl->clear();
   }

   void GameplayFacade::addTerrainCreatedListener(const cm::TerrainCreatedFn& func) {
      impl->addTerrainCreatedListener(func);
   }

}
