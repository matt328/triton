#pragma once

namespace Triton {

   namespace Actions {
      class ActionManager;
   }

   class ApplicationContext {
    public:
      explicit ApplicationContext(int width, int height, const std::string_view& windowTitle);
      ~ApplicationContext();

      ApplicationContext(const ApplicationContext&) = delete;
      ApplicationContext(ApplicationContext&&) = delete;
      ApplicationContext& operator=(const ApplicationContext&) = delete;
      ApplicationContext& operator=(ApplicationContext&&) = delete;

      void start();

      [[nodiscard]] std::shared_ptr<Actions::ActionManager> getActionManager();

    private:
      class ApplicationContextImpl;
      std::unique_ptr<ApplicationContextImpl> impl;
   };

};
