#pragma once

namespace tr {

class IEntityManager {
public:
  IEntityManager() = default;
  virtual ~IEntityManager() = default;

  IEntityManager(const IEntityManager&) = default;
  IEntityManager(IEntityManager&&) = delete;
  auto operator=(const IEntityManager&) -> IEntityManager& = default;
  auto operator=(IEntityManager&&) -> IEntityManager& = delete;

  virtual auto update() -> void = 0;
};

}
