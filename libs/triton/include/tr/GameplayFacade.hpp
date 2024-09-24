#pragma once

#include "cm/EntitySystemTypes.hpp"

namespace tr::gp {
   class GameplaySystem;
} // namespace tr::gp

namespace tr::gfx {
   class RenderContext;
} // namespace tr::gfx

namespace tr::ctx {
   template <typename T>
   using OptionalRef = std::optional<std::reference_wrapper<T>>;

   struct EditorInfoComponent {
      std::string name{};
      std::optional<std::string> sourceMesh{};
      std::optional<std::string> sourceTexture{};
   };

   /*
      This class is a facade over the gameplay api, and the ResourceManager.
      It ensures that as game world objects are created, required gfx resources are created and
      linked to.

      This class should also live in the intermediate module between gp and gfx.
   */
   class GameplayFacade {
    public:
      GameplayFacade(gp::GameplaySystem& gameplaySystem, gfx::RenderContext& renderer);
      ~GameplayFacade();

      GameplayFacade(const GameplayFacade&) = delete;
      GameplayFacade(GameplayFacade&&) = delete;
      GameplayFacade& operator=(const GameplayFacade&) = delete;
      GameplayFacade& operator=(GameplayFacade&&) = delete;

      // Entity Creation Methods
      [[nodiscard]] auto getCreateTerrainFn() const -> std::function<cm::EntityType(uint32_t)>;

      [[nodiscard]] auto createCamera(uint32_t width,
                                      uint32_t height,
                                      float fov,
                                      float zNear,
                                      float zFar,
                                      const glm::vec3& position,
                                      const std::optional<std::string>& name = std::nullopt) const
          -> cm::EntityType;

      auto setCurrentCamera(cm::EntityType currentCamera) const -> void;

      [[nodiscard]] auto getStaticModelEntityTask() const noexcept
          -> std::function<cm::EntityType(const std::filesystem::path&)>;

      [[nodiscard]] auto getAnimatedModelEntityTask() const
          -> std::function<cm::EntityType(const std::filesystem::path& modelPath,
                                          const std::filesystem::path& skeletonPath,
                                          const std::filesystem::path& animationPath)>;

      auto clear() const -> void;

    private:
      class Impl;
      std::unique_ptr<Impl> impl;
   };

} // namespace tr::ctx
