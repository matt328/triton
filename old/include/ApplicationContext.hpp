#pragma once

#include "Events.hpp"
namespace Triton {

   namespace Actions {
      class ActionManager;
   }

   class ResourceFactory;
   struct RenderObject;
   struct PerFrameData;

   // TODO: pimpl is seriously a massive PITA that's twice now, just get rid of it and forget
   // about it.

   class ApplicationContext {
    public:
      explicit ApplicationContext(int width, int height, const std::string_view& windowTitle);
      ~ApplicationContext();

      ApplicationContext(const ApplicationContext&) = delete;
      ApplicationContext(ApplicationContext&&) = delete;
      ApplicationContext& operator=(const ApplicationContext&) = delete;
      ApplicationContext& operator=(ApplicationContext&&) = delete;

      void start();

      [[nodiscard]] std::shared_ptr<Actions::ActionManager> createActionManager();
      [[nodiscard]] std::shared_ptr<ResourceFactory> createResourceFactory(
          std::filesystem::path rootPath);

      void registerRenderObjectProvider(std::function<std::vector<RenderObject>()> fn);
      void registerPerFrameDataProvider(std::function<PerFrameData()> fn);

      size_t addEventHandler(std::function<void(Events::Event&)> fn);

    private:
      class ApplicationContextImpl;
      std::unique_ptr<ApplicationContextImpl> impl;
   };

};
