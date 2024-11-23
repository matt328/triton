#pragma once
#include <gp/Registry.hpp>

namespace tr::gp::sys {
   class TransformSystem {
    public:
      TransformSystem(std::shared_ptr<Registry> newRegistry);
      ~TransformSystem() = default;

      TransformSystem(const TransformSystem&) = delete;
      TransformSystem& operator=(const TransformSystem&) = delete;
      TransformSystem(TransformSystem&&) = delete;
      TransformSystem& operator=(TransformSystem&&) = delete;

      auto update() const -> void;

    private:
      std::shared_ptr<Registry> registry;
   };
}