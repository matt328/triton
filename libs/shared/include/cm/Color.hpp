#include <glm/glm.hpp>

namespace tr {
struct Color {
  glm::vec3 rgb;

  constexpr Color(float r, float g, float b) : rgb(r, g, b) {
  }

  constexpr auto operator==(const Color& other) const -> bool = default;
};

namespace Colors {
static constexpr Color Red{1.0f, 0.0f, 0.0f};
static constexpr Color Green{0.0f, 1.0f, 0.0f};
static constexpr Color Blue{0.0f, 0.0f, 1.0f};
static constexpr Color Yellow{1.0f, 1.0f, 0.0f};
static constexpr Color Cyan{0.0f, 1.0f, 1.0f};
static constexpr Color Magenta{1.0f, 0.0f, 1.0f};
static constexpr Color White{1.0f, 1.0f, 1.0f};
static constexpr Color Black{0.0f, 0.0f, 0.0f};
static constexpr Color Gray{0.5f, 0.5f, 0.5f};
static constexpr Color Orange{1.0f, 0.5f, 0.0f};
static constexpr Color Purple{0.5f, 0.0f, 0.5f};
static constexpr Color Brown{0.6f, 0.3f, 0.1f};
static constexpr Color Pink{1.0f, 0.75f, 0.8f};
}
}
