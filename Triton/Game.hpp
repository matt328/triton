#pragma once
#include <entt/entt.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <memory>

struct Transform {
  explicit Transform(const glm::mat4 transform) : transform(transform) {}
  glm::mat4 transform;

  explicit operator glm::mat4() const { return transform; }

  glm::vec3 getPosition() const {
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    decompose(transform, scale, rotation, translation, skew, perspective);
    return translation;
  }
};

class Game {
 public:
  Game();
  ~Game() = default;

  void update(double t, float dt) const;
  void blendState(double alpha);

 private:
  std::unique_ptr<entt::registry> registry;
};
