#pragma once

#include "components/AppLog.hpp"
#include "ui/components/Menu.hpp"

namespace ed::ui {

   class Manager {
    public:
      explicit Manager(std::shared_ptr<cmp::Menu> newAppMenu);
      ~Manager();

      Manager(const Manager&) = delete;
      Manager(Manager&&) = delete;
      auto operator=(const Manager&) -> Manager& = delete;
      auto operator=(Manager&&) -> Manager& = delete;

      void render();

    private:
      ImFont* sauce = nullptr;
      std::shared_ptr<cmp::AppLog> appLog;
      std::shared_ptr<cmp::Menu> appMenu;

      auto setupFonts() -> void;
   };

}
