#pragma once

#include <entt/entt.hpp>

#include "cm/Handles.hpp"

#include "Application.hpp"
#include "data/DataFacade.hpp"

#include "components/EntityEditor.hpp"

namespace ed {
   class Application;
}

namespace tr::ctx {
   class GameplayFacade;
}

namespace tr::util {
   class TaskQueue;
}

namespace ed::ui {
   class Manager {
    public:
      Manager(tr::ctx::GameplayFacade& facade, data::DataFacade& dataFacade);
      ~Manager();

      Manager(const Manager&) = delete;
      Manager(Manager&&) = delete;
      Manager& operator=(const Manager&) = delete;
      Manager& operator=(Manager&&) = delete;

      void render();

      template <auto Candidate, typename Type>
      void addQuitListener(Type* v) noexcept {
         quitDelegate.connect<Candidate>(v);
      }

      void setFullscreenFn(std::function<void(void)> fn) {
         this->toggleFullscreenFn = fn;
      }

      void setWireframeFn(std::function<void(bool)> fn) {
         this->wireframeCallback = fn;
      }

    private:
      static constexpr auto ProjectFileFilters =
          std::array<nfdfilteritem_t, 1>{nfdfilteritem_t{"Triton Project", "trp"}};

      tr::ctx::GameplayFacade& facade;
      data::DataFacade& dataFacade;

      components::EntityEditor entityEditor;

      std::vector<std::future<tr::cm::ModelHandle>> modelFutures{};
      std::vector<std::future<tr::cm::ModelHandle>> terrainFutures{};
      std::vector<std::future<tr::cm::LoadedSkinnedModelData>> skinnedModelFutures{};

      // HACK: get this from the application somehow instead of tracking it in 2 places
      bool fullscreen{};

      std::optional<uint32_t> selectedEntity{};
      std::optional<std::filesystem::path> openFilePath{};

      entt::delegate<void(void)> quitDelegate{};

      std::function<void(void)> toggleFullscreenFn;
      std::function<void(bool)> wireframeCallback;
      bool enableWireframe{false};

      std::unique_ptr<NFD::Guard> guard;

      void handleModelFutures();
      void handleTerrainFutures();
      void handleSkinnedModelFutures();

      void renderDockSpace();
      void renderMenuBar();
      void renderEntityEditor();
      void showMatrix4x4(const glm::mat4& matrix, const char* label);
      void renderAnimationArea(tr::gp::ecs::Animation& animationComponent);

      void renderDebugWindow();

      auto getSavePath() -> std::optional<std::filesystem::path>;
   };
}