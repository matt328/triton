#pragma once

#include <memory>
#include <string>
#include <functional>

#include "IGame.hpp"
#include "ResourceFactory.hpp"

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

   void registerGame(std::shared_ptr<IGame> game);

   void deregisterUpdate(const size_t num);
   void deregisterUpdateBlendState(const size_t num);
   void deregisterKeyHandler(const size_t num);

 private:
   class ApplicationImpl;
   std::unique_ptr<ApplicationImpl> impl;
};
