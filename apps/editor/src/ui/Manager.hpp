#pragma once

#include <entt/entt.hpp>

#include "cm/Handles.hpp"
#include "data/DataFacade.hpp"
#include "components/EntityEditor.hpp"
#include "ui/components/ModalDialog.hpp"

namespace ed {
   class Application;
   namespace ui::cmp {
      struct AppLog;
   }
}

namespace tr::ctx {
   class GameplayFacade;
}

namespace ed::ui {
   class Manager {
    public:
      Manager(tr::ctx::GameplayFacade& facade, data::DataFacade& dataFacade);
      ~Manager();

      Manager(const Manager&) = delete;
      Manager(Manager&&) = delete;
      auto operator=(const Manager&) -> Manager& = delete;
      auto operator=(Manager&&) -> Manager& = delete;

      void render();

      template <auto Candidate, typename Type>
      void addQuitListener(Type* v) noexcept {
         quitDelegate.connect<Candidate>(v);
      }

      void setFullscreenFn(const std::function<void()>& fn) {
         this->toggleFullscreenFn = fn;
      }

      void setWireframeFn(const std::function<void(bool)>& fn) {
         this->wireframeCallback = fn;
      }

    private:
      static constexpr auto ProjectFileFilters =
          std::array{nfdfilteritem_t{"Triton Project", "trp"}};

      std::unordered_map<std::string, std::unique_ptr<cmp::ModalDialog>> dialogMap;

      tr::ctx::GameplayFacade& facade;
      data::DataFacade& dataFacade;

      components::EntityEditor entityEditor;

      // HACK: get this from the application somehow instead of tracking it in 2 places
      bool fullscreen{};

      std::optional<uint32_t> selectedEntity{};
      std::optional<std::filesystem::path> openFilePath{};

      entt::delegate<void()> quitDelegate{};

      std::function<void()> toggleFullscreenFn;
      std::function<void(bool)> wireframeCallback;
      bool enableWireframe{false};

      std::unique_ptr<NFD::Guard> guard;

      ImFont* sauce = nullptr;

      std::unique_ptr<cmp::AppLog> appLog;

      void handleTerrainFutures();
      void handleSkinnedModelFutures();

      static void renderDockSpace();
      void renderMenuBar();
      void renderEntityEditor();
      static void showMatrix4x4(const glm::mat4& matrix, const char* label);
      void renderAnimationArea();

      void renderDebugWindow();

      static auto getSavePath() -> std::optional<std::filesystem::path>;
   };
}