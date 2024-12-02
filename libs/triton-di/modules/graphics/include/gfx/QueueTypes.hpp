#pragma once
#include <vk/Device.hpp>

namespace tr::gfx::queue {
   class Graphics {
    public:
      explicit Graphics(const std::shared_ptr<Device>& device);
      [[nodiscard]] auto getQueue() const -> vk::raii::Queue&;

    private:
      std::unique_ptr<vk::raii::Queue> queue;
   };

   class Present {
    public:
      explicit Present(const std::shared_ptr<Device>& device);
      [[nodiscard]] auto getQueue() const -> vk::raii::Queue&;

    private:
      std::unique_ptr<vk::raii::Queue> queue;
   };

   class Transfer {
    public:
      explicit Transfer(const std::shared_ptr<Device>& device);
      [[nodiscard]] auto getQueue() const -> vk::raii::Queue&;

    private:
      std::unique_ptr<vk::raii::Queue> queue;
   };

   class Compute {
    public:
      explicit Compute(const std::shared_ptr<Device>& device);
      [[nodiscard]] auto getQueue() const -> vk::raii::Queue&;

    private:
      std::unique_ptr<vk::raii::Queue> queue;
   };
}