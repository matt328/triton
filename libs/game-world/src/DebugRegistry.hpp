#pragma once

#include "api/fx/IDebugRegistry.hpp"

/*
  TODO(matt): Implement immediate mode buffer handling. use a persistently cpu mapped buffer and
  rewrite the entire contents of it every frame.
  1 Change DebugRegistry back to being a singleton.
  2 create a DebugProcessor component to update once per frame and triangulate and queue up for
      rendering any shapes that should be rendered this frame.
  3 the DebugRegistry should have an update method so it can timeout any shapes that had duration

  todo tomorrow: remove IDebugRegistry from all the DI areas.

*/

namespace tr {

class IResourceProxy;

class DebugTools : public IDebugRegistry {
public:
  explicit DebugTools(std::shared_ptr<IResourceProxy> newResourceProxy);
  ~DebugTools() override;

  DebugTools(const DebugTools&) = default;
  DebugTools(DebugTools&&) = delete;
  auto operator=(const DebugTools&) -> DebugTools& = default;
  auto operator=(DebugTools&&) -> DebugTools& = delete;

  auto addDebugCube(std::string tag, glm::vec3 center, float extent, Color color) -> void override;

  auto clearTag(const std::string& tag) -> void override;

  auto clearAll() -> void override;

  [[nodiscard]] auto getShapes() const -> std::vector<std::shared_ptr<IShape>> override;

  auto setDirty(bool newDirty) -> void override;

  [[nodiscard]] auto isDirty() const -> bool override;

private:
  std::shared_ptr<IResourceProxy> resourceProxy;

  bool dirty{};
  std::multimap<std::string, std::shared_ptr<IShape>> shapeMap;
};

}
