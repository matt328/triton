#pragma once

#include "components/AppLog.hpp"

namespace ed::ui {

   class Manager {
    public:
      Manager();
      ~Manager() = default;

      Manager(const Manager&) = delete;
      Manager(Manager&&) = delete;
      auto operator=(const Manager&) -> Manager& = delete;
      auto operator=(Manager&&) -> Manager& = delete;

      void render();

    private:
      ImFont* sauce = nullptr;
      std::unique_ptr<cmp::AppLog> appLog;

      auto setupFonts() -> void;
   };

}
