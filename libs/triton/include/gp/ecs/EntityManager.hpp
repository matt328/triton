#pragma once

namespace tr::gp::ecs {
   class EntityManager {
    public:
      EntityManager();
      ~EntityManager();

      EntityManager(const EntityManager&) = default;
      EntityManager& operator=(const EntityManager&) = default;

      EntityManager(EntityManager&&) = delete;
      EntityManager& operator=(EntityManager&&) = delete;

    private:
   };
}
