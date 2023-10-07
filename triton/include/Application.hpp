#pragma once

#include <memory>
#include <string>
#include <functional>

#include "ResourceFactory.hpp"
#include "Events.hpp"
#include "TransferData.hpp"

class Application {
 public:
   Application(int width, int height, const std::string_view& windowTitle);
   ~Application();

   Application(const Application&) = delete;
   Application(Application&&) = delete;
   Application& operator=(const Application&) = delete;
   Application& operator=(Application&&) = delete;

   void run() const;

   IResourceFactory* getResourceFactory();

   size_t registerUpdate(std::function<void(void)> fn);
   size_t registerUpdateBlendState(std::function<void(double)> fn);
   size_t registerKeyHandler(std::function<void(int, int, int, int)> fn);

   void deregisterUpdate(const size_t num);
   void deregisterUpdateBlendState(const size_t num);
   void deregisterKeyHandler(const size_t num);

   void setEventCallbackFn(std::function<void(Events::Event&)> fn);
   void registerRenderObjectProvider(std::function<std::vector<RenderObject>()> fn);
   void registerPerFrameDataProvider(std::function<PerFrameData()> fn);

 private:
   class ApplicationImpl;
   std::unique_ptr<ApplicationImpl> impl;
};
