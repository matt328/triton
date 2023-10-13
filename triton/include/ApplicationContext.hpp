#pragma once

namespace Triton {

   class ApplicationContext {
    public:
      explicit ApplicationContext(int width, int height, const std::string_view& windowTitle);
      ~ApplicationContext();

      ApplicationContext(const ApplicationContext&) = delete;
      ApplicationContext(ApplicationContext&&) = delete;
      ApplicationContext& operator=(const ApplicationContext&) = delete;
      ApplicationContext& operator=(ApplicationContext&&) = delete;

      void start();

    private:
      class ApplicationContextImpl;
      std::unique_ptr<ApplicationContextImpl> impl;
   };

};
