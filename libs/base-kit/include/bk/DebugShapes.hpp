#pragma once

#include "bk/Color.hpp"

namespace tr {
enum class GizmoMode : uint8_t {
  Translate = 0u,
  Rotate,
  Scale
};

struct IShape {
  explicit IShape(Color newColor, float newDuration) : color{newColor}, duration{newDuration} {
  }
  IShape(const IShape&) = default;
  IShape(IShape&&) = delete;
  auto operator=(const IShape&) -> IShape& = default;
  auto operator=(IShape&&) -> IShape& = delete;

  virtual ~IShape() = default;
  virtual auto getPoints() -> std::vector<glm::vec3> = 0;
  Color color = Colors::White;
  float duration{};
};

struct Line : public IShape {
  Line() = delete;
  Line(glm::vec3 newStart, glm::vec3 newEnd, Color newColor, float newDuration)
      : IShape{newColor, newDuration}, start{newStart}, end{newEnd} {
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
  explicit Cube(std::vector<Line> newLines, Color newColor, float newDuration)
      : IShape{newColor, newDuration}, lines{std::move(newLines)} {
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
}
