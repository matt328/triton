#pragma once
#include <vk/Device.hpp>

namespace tr::gfx::queue {
   class Graphics {
    public:
      explicit Graphics(const std::shared_ptr<Device>& device);
      [[nodiscard]] auto getQueue() const -> vk::raii::Queue&;
      [[nodiscard]] auto getFamily() const -> uint32_t;

    private:
      std::shared_ptr<vk::raii::Queue> queue;
      uint32_t queueFamilyIndex;
   };

   class Present {
    public:
      explicit Present(const std::shared_ptr<Device>& device);
      [[nodiscard]] auto getQueue() const -> vk::raii::Queue&;
      [[nodiscard]] auto getFamily() const -> uint32_t;

    private:
      std::unique_ptr<vk::raii::Queue> queue;
      uint32_t queueFamilyIndex;
   };

   class Transfer {
    public:
      explicit Transfer(const std::shared_ptr<Device>& device);
      [[nodiscard]] auto getQueue() const -> vk::raii::Queue&;
      [[nodiscard]] auto getFamily() const -> uint32_t;

    private:
      std::unique_ptr<vk::raii::Queue> queue;
      uint32_t queueFamilyIndex;
   };

   class Compute {
    public:
      explicit Compute(const std::shared_ptr<Device>& device);
      [[nodiscard]] auto getQueue() const -> vk::raii::Queue&;
      [[nodiscard]] auto getFamily() const -> uint32_t;

    private:
      std::unique_ptr<vk::raii::Queue> queue;
      uint32_t queueFamilyIndex;
   };
}
