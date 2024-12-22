#pragma once
#include "Instance.hpp"
#include <tr/IWindow.hpp>

namespace tr {

   class Surface {
    public:
      Surface(std::shared_ptr<IWindow> newWindow, std::shared_ptr<Instance> newInstance);
      ~Surface();

      Surface(const Surface&) = delete;
      Surface(Surface&&) = delete;
      auto operator=(const Surface&) -> Surface& = delete;
      auto operator=(Surface&&) -> Surface& = delete;

      auto getVkSurface() const -> vk::raii::SurfaceKHR&;

    private:
      std::shared_ptr<IWindow> window;
      std::shared_ptr<Instance> instance;

      std::unique_ptr<vk::raii::SurfaceKHR> surface;
   };

}
