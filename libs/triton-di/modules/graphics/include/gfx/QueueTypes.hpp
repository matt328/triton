#pragma once

namespace tr::gfx::queue {
   class Graphics {
    public:
      explicit Graphics(std::unique_ptr<vk::raii::Queue> newQueue);

      [[nodiscard]] auto getQueue() const -> vk::raii::Queue&;

    private:
      std::unique_ptr<vk::raii::Queue> queue;
   };

   class Present {
    public:
      explicit Present(std::unique_ptr<vk::raii::Queue> newQueue);

    private:
      std::unique_ptr<vk::raii::Queue> queue;
   };

   class Transfer {
    public:
      explicit Transfer(std::unique_ptr<vk::raii::Queue> newQueue);

    private:
      std::unique_ptr<vk::raii::Queue> queue;
   };

   class Compute {
    public:
      explicit Compute(std::unique_ptr<vk::raii::Queue> newQueue);

    private:
      std::unique_ptr<vk::raii::Queue> queue;
   };
}