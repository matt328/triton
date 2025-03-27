#pragma once
#include "api/EntitySystemTypes.hpp"
#include "api/Color.hpp"

namespace tr {

enum class GizmoMode : uint8_t {
  Translate = 0u,
  Rotate,
  Scale
};

struct IShape {
  explicit IShape(Color newColor) : color{newColor} {
  }
  IShape(const IShape&) = default;
  IShape(IShape&&) = delete;
  auto operator=(const IShape&) -> IShape& = default;
  auto operator=(IShape&&) -> IShape& = delete;

  virtual ~IShape() = default;
  virtual auto getPoints() -> std::vector<glm::vec3> = 0;
  Color color = Colors::White;
};

struct Line : public IShape {
  Line() = delete;
  Line(glm::vec3 newStart, glm::vec3 newEnd, Color newColor)
      : IShape{newColor}, start{newStart}, end{newEnd} {
  }
  glm::vec3 start;
  glm::vec3 end;

  auto getPoints() -> std::vector<glm::vec3> override {
    return {start, end};
  }
};

struct Triangle : public IShape {
  std::array<Line, 3> lines;
  auto getPoints() -> std::vector<glm::vec3> override {
    auto points = std::vector<glm::vec3>{};
    for (const auto& line : lines) {
      points.push_back(line.start);
      points.push_back(line.end);
    }
    return points;
  }
};

struct Cube : public IShape {
  explicit Cube(std::vector<Line> newLines, Color newColor)
      : IShape{newColor}, lines{std::move(newLines)} {
  }

  std::vector<Line> lines;

  auto getPoints() -> std::vector<glm::vec3> override {
    auto points = std::vector<glm::vec3>{};
    for (const auto& line : lines) {
      points.push_back(line.start);
      points.push_back(line.end);
    }
    return points;
  }
};

struct Label {
  glm::vec3 position;
  std::string label;
};

class IDebugService {
public:
  IDebugService() = default;
  virtual ~IDebugService() = default;

  IDebugService(const IDebugService&) = default;
  IDebugService(IDebugService&&) = delete;
  auto operator=(const IDebugService&) -> IDebugService& = default;
  auto operator=(IDebugService&&) -> IDebugService& = delete;

  virtual auto addDebugCube(std::string tag,
                            glm::vec3 center,
                            float extent,
                            Color color) -> void = 0;
  virtual auto addDebugLine(std::string tag, glm::vec3 start, glm::vec3 end) -> void = 0;

  virtual auto labelPoint(std::string tag, glm::vec3 point, std::string label) -> void = 0;
  virtual auto labelLine(std::string tag,
                         glm::vec3 start,
                         glm::vec3 end,
                         std::string label) -> void = 0;
  virtual auto labelTriangle(std::string tag,
                             std::array<glm::vec3, 3> triangle,
                             std::string label) -> void = 0;

  virtual auto clearTag(std::string tag) -> void = 0;
  virtual auto clearAll() -> void = 0;
  virtual auto setGizmoMode() -> void = 0;
  virtual auto addGizmoToEntity(tr::EntityType entityId) -> void = 0;

  [[nodiscard]] virtual auto getShapes() const -> std::vector<std::shared_ptr<IShape>> = 0;

  virtual auto setDirty(bool newDirty) -> void = 0;
  [[nodiscard]] virtual auto isDirty() const -> bool = 0;
};

}
