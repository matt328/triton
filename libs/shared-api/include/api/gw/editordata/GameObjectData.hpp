#pragma once

namespace tr {

struct Orientation {
  glm::vec3 position{glm::zero<glm::vec3>()};
  glm::vec3 rotation{glm::zero<glm::vec3>()};

  template <class T>
  void serialize(T& archive) {
    archive(position, rotation);
  }
};

struct GameObjectData {
  std::string name;
  Orientation orientation;

  std::string modelName;
  std::string skeleton;
  std::vector<std::string> animations;

  glm::vec3 camPosition{glm::zero<glm::vec3>()};
  float yaw{0.f};
  float pitch{0.f};

  template <class T>
  void serialize(T& archive) {
    archive(name, orientation, modelName, skeleton, animations, camPosition, yaw, pitch);
  }
};
}
