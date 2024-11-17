#pragma once

#include "Properties.hpp"
#include "data/DataFacade.hpp"
#include "ui/components/DialogManager.hpp"

namespace ed::ui::cmp {

   class Menu {
    public:
      explicit Menu(std::shared_ptr<data::DataFacade> newDataFacade,
                    std::shared_ptr<Properties> newProperties,
                    std::shared_ptr<DialogManager> newDialogManager);
      ~Menu() = default;

      Menu(const Menu&) = default;
      Menu(Menu&&) = delete;
      auto operator=(const Menu&) -> Menu& = default;
      auto operator=(Menu&&) -> Menu& = delete;

      void render();

      void setQuitFn(const std::function<void(void)>& newQuitFn) {
         quitFn = newQuitFn;
      }

      void setFullscreenFn(std::function<void(void)>& newFullscreenFn) {
         toggleFullscreenFn = newFullscreenFn;
      }

    private:
      std::shared_ptr<data::DataFacade> dataFacade;
      std::shared_ptr<Properties> properties;
      std::shared_ptr<DialogManager> dialogManager;

      std::optional<std::filesystem::path> openFilePath{};
      bool fullscreen{};
      bool enableWireframe{};

      std::function<void(void)> quitFn;
      std::function<void(void)> toggleFullscreenFn;

      static constexpr auto ProjectFileFilters =
          std::array{nfdfilteritem_t{"Triton Project", "trp"}};

      static auto getSavePath() -> std::optional<std::filesystem::path>;
   };
}
