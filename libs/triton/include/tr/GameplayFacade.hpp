#pragma once

#include "cm/Handles.hpp"
#include "cm/EntitySystemTypes.hpp"

namespace tr::gp {
   class GameplaySystem;
}

namespace tr::gfx {
   class RenderContext;
}

namespace tr::ctx {
   template <typename T>
   using OptionalRef = std::optional<std::reference_wrapper<T>>;

   struct EditorInfoComponent {
      std::string name;
      std::optional<std::string> sourceMesh;
      std::optional<std::string> sourceTexture;
   };

   /*
      This class is a facade over the gameplay api, and the ResourceManager.
      It ensures that as game world objects are created, required gfx resources are created and
      linked to.

      This class should also live in the intermediate module between gp and gfx.
   */
   class GameplayFacade {
    public:
      GameplayFacade(gp::GameplaySystem& gameplaySystem,
                     gfx::RenderContext& renderer,
                     bool debugEnabled = false);
      ~GameplayFacade();

      GameplayFacade(const GameplayFacade&) = delete;
      GameplayFacade(GameplayFacade&&) = delete;
      GameplayFacade& operator=(const GameplayFacade&) = delete;
      GameplayFacade& operator=(GameplayFacade&&) = delete;

      // Entity Creation Methods
      auto createSkinnedModelEntity(const cm::LoadedSkinnedModelData model) -> cm::EntityType;

      auto createStaticMultiMeshEntity(const cm::MeshHandles meshes) -> cm::EntityType;

      auto createTerrain(const uint32_t size) -> futures::cfuture<cm::EntityType>;

      auto createCamera(uint32_t width,
                        uint32_t height,
                        float fov,
                        float zNear,
                        float zFar,
                        glm::vec3 position,
                        std::optional<std::string> name = std::nullopt) -> cm::EntityType;

      auto setCurrentCamera(cm::EntityType currentCamera) -> void;

      auto createStaticModelEntity(const std::filesystem::path& modelPath)
          -> futures::cfuture<cm::EntityType>;

      auto createAnimatedModelEntity(const std::filesystem::path& modelPath,
                                     const std::filesystem::path& skeletonPath,
                                     const std::filesystem::path& animationPath)
          -> futures::cfuture<cm::EntityType>;

      auto clear() -> void;

      // Resource Creation Methods
      /*
         TODO:
         This facade shouldn't expose the two parts of gfx loading and then gp creation. It should
         just be able to tell the gp system to create something, then the gp system should be able
         to tell the gfx system what assets it should use to render the entity.

         In a game application, these assets might be loaded differently, from a pack or something,
         so we'll need to think about what API the editor needs vs what API the game needs, and how
         to factor out a common API that both of those can talk to.
      */
      auto loadModelAsync(const std::filesystem::path& path) -> std::future<cm::ModelHandle>;

      auto loadSkinnedModelAsync(const std::filesystem::path& modelPath,
                                 const std::filesystem::path& skeletonPath,
                                 const std::filesystem::path& animationPath)
          -> std::future<cm::LoadedSkinnedModelData>;

      void loadModelResources(const std::filesystem::path& modelPath,
                              const std::filesystem::path& skeletonPath,
                              const std::filesystem::path& animationPath,
                              const std::function<void()> done);

      void update();

    private:
      class Impl;
      std::unique_ptr<Impl> impl;
   };

}
