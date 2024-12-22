#include "Surface.hpp"

namespace tr {
   Surface::Surface(std::shared_ptr<IWindow> newWindow, std::shared_ptr<Instance> newInstance)
       : window{std::move(newWindow)}, instance{std::move(newInstance)} {
      surface = instance->createSurface(*window);
   }
   Surface::~Surface() {
      Log.trace("Destroying Surface");
   }
   auto Surface::getVkSurface() const -> vk::raii::SurfaceKHR& {
      return *surface;
   }
}