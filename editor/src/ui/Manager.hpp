#pragma once

#include "Application.hpp"
#include "gp/ecs/component/Animation.hpp"
#include "gfx/Handles.hpp"
#include <entt/entt.hpp>

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
      Manager(tr::ctx::GameplayFacade& facade);
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
      tr::ctx::GameplayFacade& facade;

      std::vector<std::future<tr::gfx::ModelHandle>> modelFutures{};
      std::vector<std::future<tr::gfx::ModelHandle>> terrainFutures{};
      std::vector<std::future<tr::gfx::LoadedSkinnedModelData>> skinnedModelFutures{};

      // HACK: get this from the application somehow instead of tracking it in 2 places
      bool fullscreen{};

      std::optional<uint32_t> selectedEntity{};
      std::optional<std::filesystem::path> openFilePath{};
      bool dirty = false;

      ImGui::FileBrowser openProjectFileDialog;
      ImGui::FileBrowser saveProjectFileDialog{ImGuiFileBrowserFlags_EnterNewFilename |
                                               ImGuiFileBrowserFlags_CreateNewDir};

      entt::delegate<void(void)> quitDelegate{};

      std::function<void(void)> toggleFullscreenFn;
      std::function<void(bool)> wireframeCallback;
      bool enableWireframe{false};

      void handleModelFutures();
      void handleTerrainFutures();
      void handleSkinnedModelFutures();

      void renderDockSpace();
      void renderMenuBar();
      void renderEntityEditor();
      void renderDialogs();
      void showMatrix4x4(const glm::mat4& matrix, const char* label);
      void renderAnimationArea(tr::gp::ecs::Animation& animationComponent);

      void renderDebugWindow();
   };
}