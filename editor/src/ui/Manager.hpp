#pragma once

#include "Application.hpp"
#include <entt/entt.hpp>

namespace ed {
   class Application;
}

namespace tr::ctx {
   class GameplayFacade;
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

    private:
      tr::ctx::GameplayFacade& facade;

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

      void renderDockSpace();
      void renderMenuBar();
      void renderEntityEditor();
      void renderDialogs();
   };
}