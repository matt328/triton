#pragma once

#include "EntityService.hpp"

namespace tr {

class IEntityManager;

class TransformSystem {
public:
  explicit TransformSystem(std::shared_ptr<EntityService> newEntityService);
  ~TransformSystem() = default;

  TransformSystem(const TransformSystem&) = delete;
  auto operator=(const TransformSystem&) -> TransformSystem& = delete;
  TransformSystem(TransformSystem&&) = delete;
  auto operator=(TransformSystem&&) -> TransformSystem& = delete;

  auto update() -> void;

private:
  std::shared_ptr<IEntityManager> entityService;
};

}
