#pragma once

#include "tr/IDebugService.hpp"

namespace tr {

class DefaultDebugService : public IDebugService {
public:
  auto addDebugCube(std::string tag, glm::vec3 center, float extent, Color color) -> void override;
  auto addDebugLine(std::string tag, glm::vec3 start, glm::vec3 end) -> void override;
  auto labelPoint(std::string tag, glm::vec3 point, std::string label) -> void override;
  auto labelLine(std::string tag,
                 glm::vec3 start,
                 glm::vec3 end,
                 std::string label) -> void override;
  auto labelTriangle(std::string tag,
                     std::array<glm::vec3, 3> triangle,
                     std::string label) -> void override;
  auto clearTag(std::string tag) -> void override;
  auto clearAll() -> void override;
  auto setGizmoMode() -> void override;
  auto addGizmoToEntity(tr::EntityType entityId) -> void override;
  [[nodiscard]] auto getShapes() const -> std::vector<std::shared_ptr<IShape>> override;
  auto setDirty(bool newDirty) -> void override;
  [[nodiscard]] auto isDirty() const -> bool override;

private:
  bool dirty{};
  std::multimap<std::string, std::shared_ptr<IShape>> shapeMap;
};

}
