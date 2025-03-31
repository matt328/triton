#pragma once

#include "bk/Color.hpp"

namespace tr {

struct IShape;

class IDebugRegistry {
public:
  IDebugRegistry() = default;
  virtual ~IDebugRegistry() = default;

  IDebugRegistry(const IDebugRegistry&) = default;
  IDebugRegistry(IDebugRegistry&&) = delete;
  auto operator=(const IDebugRegistry&) -> IDebugRegistry& = default;
  auto operator=(IDebugRegistry&&) -> IDebugRegistry& = delete;

  virtual auto addDebugCube(std::string tag, glm::vec3 center, float extent, Color color)
      -> void = 0;

  virtual auto clearTag(const std::string& tag) -> void = 0;

  virtual auto clearAll() -> void = 0;

  [[nodiscard]] virtual auto getShapes() const -> std::vector<std::shared_ptr<IShape>> = 0;

  virtual auto setDirty(bool newDirty) -> void = 0;

  virtual auto isDirty() const -> bool = 0;
};

}
