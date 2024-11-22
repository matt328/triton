#pragma once

#include <entt/entity/fwd.hpp>

namespace tr::gp {
   class Registry {
    public:
      Registry();
      ~Registry();

      Registry(const Registry&) = delete;
      Registry(Registry&&) = delete;
      auto operator=(const Registry&) -> Registry& = delete;
      auto operator=(Registry&&) -> Registry& = delete;

      [[nodiscard]] auto getRegistry() const -> entt::registry&;
      [[nodiscard]] auto getConstRegistry() const -> const entt::registry&;

    private:
      std::unique_ptr<entt::registry> registry;
   };
}
